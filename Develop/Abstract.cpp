#include"Abstract.hpp"

AbsFile::AbsFile():m_lines((Line*)malloc(m_max_line*sizeof(Line))){
    for(int i=0;i<m_max_line;i++) m_isWriten[i].store(false);
}

AbsFile::~AbsFile(){
    //这么写的话在linux上没问题，但win上的clang编译之后的运行在这里会访问野指针，原因很显然啊。。如果absfile不满m_max_line行，你这么析构明显要出问题啊。
	//但是linux里为什么不出问题我就不知道了。。
	//for (int i = 0; i < m_max_line; i++) m_lines[i].~Line();
    free(m_lines);
}

bool AbsFile::push_to_absfile(ZLogLine&& _zline,size_t cur_line){
    if(cur_line>m_max_line-1) return true;//当前写入行比最大行数大，当前行就写不进去了！（应该不可能出现这种情况）保险起见写上，防止溢出崩溃，一旦出现了不会崩溃但会丢失一条日志
    new(&m_lines[cur_line]) Line(std::move(_zline));//???搞懂他！为什么只有这么写可以啊？？？？
    m_isWriten[cur_line].store(true,std::memory_order_release);
    if(cur_line==m_max_line-1) return true;//这是一个AbsFile中能记录的最后一条日志，记录完后要通知上层换一个新的AbsFile，正常的话不会丢失任一条日志
    return false;
}

bool AbsFile::pop_to_abslog(ZLogLine& _zline, size_t cur_line){
    //断点
    // bool isw=m_isWriten[0].load(std::memory_order_acquire);
    //如果当前指定的行数还没有被push日志行，则说明不能pop此行
    if(!m_isWriten[cur_line].load(std::memory_order_acquire)) return false;
    Line& _line=m_lines[cur_line];
    _zline=std::move(_line.m_line);
    m_isWriten[cur_line].store(false,std::memory_order_release);
    return true;
}




AbsLog::AbsLog()
:m_cur_popfile(nullptr),m_cur_pushline(0),m_cur_popline(0),m_spin{ATOMIC_FLAG_INIT}{
    set_new_pushfile();
}

void AbsLog::push_to_abslog(ZLogLine&& _zline){
    size_t cur_push=m_cur_pushline.fetch_add(1);//这里注意，必须先+1，否则多线程会出现丢失日志行！！！
    if(cur_push<AbsFile::m_max_line)
    {
        //其实这里不设置内存序也ok,默认字节序是memory_order_seq_cst（所有的顺序不许打乱）
        if(m_cur_pushfile.load()->push_to_absfile(std::move(_zline),cur_push))
        {//push_to_absfile返回true说明需要换一个AbsFile了，处理队列
            set_new_pushfile();
        }
    }
    else
    {
        //在多线程中，切换absfile会导致丢日志，这里需要如下操作：
        //当m_cur_pushline>=32768，说明此时有另一个线程正在更换absfile，
        //此时，此线程应该在此轮询等待，直到那个线程更换absfile完毕（m_cur_pushline清零）
        while(m_cur_pushline.load()>=AbsFile::m_max_line) ;//其实这相当于一个自旋锁
        //切换好absfile之后，再递归调用此函数，重新将此线程的日志行push进去
        push_to_abslog(std::move(_zline));
    }
}

//为什么pop不需要保证操作的原子性呢？是因为在ZLog中始终只有一个线程在pop日志
//而用户可能添加无数个线程来push日志
bool AbsLog::pop_to_logger(ZLogLine& _zline){
    //断点
    // int curpush=m_cur_pushline.load();
    // int qsize=m_fileq.size();
    if(!m_cur_popfile) get_next_popfile();//开始时先获得队首的absfile
    //在这里确保cur_pop不会超过最大行数
    if(m_cur_popline>AbsFile::m_max_line-1)
    {//如果超过，需get下一个popfile并更新m_cur_popline值，处理队列
        m_fileq.pop();
        m_cur_popline=0;
        get_next_popfile();
    }
    //当前行已经被push日志行了，则可以成功pop，将m_cur_popline+1，返回true
    if(m_cur_popfile && m_cur_popfile->pop_to_abslog(_zline,m_cur_popline))
    {
        m_cur_popline++;
        return true;
    }
    else return false;//如果当前行还没有push日志或者下一个popfile为空，m_cur_popline不变，需要向上通知pop失败
}
//只有这种写法才能保证在外面能正确访问m_cur_pushfile和m_fileq队列的元素
//因为在退出作用域前已经把newfile指针的控制权“移动”给了队列m_fileq中的独占指针
//所以即使退出作用域后newfile被delete掉，但一开始newfile指向的区域实际上被队列中的独占指针继续指向了，所以仍然访问得到
//当队列中的独占指针该退出作用域时，这片区域自然会被合理的释放掉，不用你操心了！
void AbsLog::set_new_pushfile(){
    std::unique_ptr<AbsFile> newfile(new AbsFile());
    m_cur_pushfile.store(newfile.get());
    spinLock lock(m_spin);//这个函数始终不存在多个线程同时进入的情况，所以考虑这个锁可以不加，实验证明ut_abs时这个锁不加没有问题
    m_fileq.push(std::move(newfile));
    m_cur_pushline.store(0);
}

void AbsLog::get_next_popfile(){
    spinLock lock(m_spin);//这个锁也是，这个函数的运行环境是单线程的，加锁又有什么用呢？？
    m_cur_popfile=m_fileq.empty() ? nullptr:m_fileq.front().get();
}