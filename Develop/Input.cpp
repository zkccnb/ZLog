#pragma warning(disable : 4996)//Clang编译器需要加上这个，屏蔽对不安全函数的报错
#include"Input.hpp"

ZLogLine::ZLogLine(ZLog::LogLevel _level,const char* _filename,const char* _funcname, size_t _line)
:m_bytes_used(0),m_cur_buffer_size(sizeof(m_stack_buffer)){
    push_to_buffer<uint64_t>(timestamp_now());//时间
    push_to_buffer<std::thread::id>(std::this_thread::get_id());//tid
    push_to_buffer<ZLog::LogLevel>(_level);//日志等级
    push_to_buffer<const char*>(_filename);//文件名
    push_to_buffer<const char*>(_funcname);//函数名
    push_to_buffer<size_t>(_line);//行号
}

void ZLogLine::pop_to_file(std::ostream& os){
    char* buffer=!m_heap_buffer ? m_stack_buffer:m_heap_buffer.get();//获取当前使用的buffer
    char* buffer_end=buffer+m_bytes_used;//buffer的结尾地址
    uint64_t time_now=*reinterpret_cast<uint64_t*>(buffer);buffer+=sizeof(uint64_t);
    std::thread::id tid=*reinterpret_cast<std::thread::id*>(buffer);buffer+=sizeof(std::thread::id);
    ZLog::LogLevel level=*reinterpret_cast<ZLog::LogLevel*>(buffer);buffer+=sizeof(ZLog::LogLevel);
    const char* filename=*reinterpret_cast<const char**>(buffer);buffer+=sizeof(const char*);
    const char* funcname=*reinterpret_cast<const char**>(buffer);buffer+=sizeof(const char*);
    size_t line=*reinterpret_cast<size_t*>(buffer);buffer+=sizeof(size_t);
    const char* level_c=nullptr;
    if(level==ZLog::LogLevel::INFO) level_c="INFO";
    if(level==ZLog::LogLevel::WARN) level_c="WARN";
    if(level==ZLog::LogLevel::CRIT) level_c="CRIT";
    format_timestamp(os, time_now);
    os<<"[tid: "<<tid<<']'
    <<'['<<level_c<<']'
    <<'['<<filename<<':'<<funcname<<':'<<line<<']'<<' ';
    if(level==ZLog::LogLevel::CRIT) os.flush();//如果日志等级为错误，则线程随时可能崩溃，这里将缓冲区残留的日志都压到文件中，以尽可能保留住关键信息
    if(buffer==buffer_end) {os<<std::endl;return;}
    int type_id=0;
    while(buffer<buffer_end){
        type_id=*reinterpret_cast<int*>(buffer);buffer+=sizeof(int);
        switch (type_id)
	    {
	    case 0:
	        continue;
	    case 1:
	        os<<*reinterpret_cast<const char**>(buffer);buffer+=sizeof(const char*);
	        continue;
	    case 2:
	        os<<*reinterpret_cast<char*>(buffer);buffer+=sizeof(char);
	        continue;
	    case 3:
	        os<<*reinterpret_cast<long long*>(buffer);buffer+=sizeof(long long);
	        continue;
	    case 4:
	        os<<*reinterpret_cast<long*>(buffer);buffer+=sizeof(long);
	        continue;
	    case 5:
	        os<<*reinterpret_cast<int*>(buffer);buffer+=sizeof(int);
	        continue;
	    case 6:
	        os<<*reinterpret_cast<double*>(buffer);buffer+=sizeof(double);
	        continue;
	    case 7:
	        os<<*reinterpret_cast<float*>(buffer);buffer+=sizeof(float);
	        continue;
	    }
    }
    os<<std::endl;
}
uint64_t ZLogLine::timestamp_now(){
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void ZLogLine::format_timestamp(std::ostream & os, uint64_t timestamp){
	std::time_t time_t = timestamp / 1000000;
	auto gmtime = std::gmtime(&time_t);
	char buffer[32];
	strftime(buffer, 32, "%Y-%m-%d %T.", gmtime);
	char microseconds[7];
	sprintf(microseconds, "%06llu", timestamp % 1000000);
	os << '[' << buffer << microseconds << ']';
}

template<typename Arg>
void ZLogLine::push_to_buffer(Arg arg){
    resize_buffer(sizeof(Arg));
    *reinterpret_cast<Arg*>(get_buffer())=arg;
    m_bytes_used+=sizeof(Arg);
}

template<typename Arg>
void ZLogLine::push_to_buffer(Arg arg, int type_id){
    // if(!type_id) std::cout<<"输入类型不对！！！"<<std::endl;
    // else std::cout<<"输入类型为："<<(int)type_id<<std::endl;
    resize_buffer(sizeof(Arg)+sizeof(int));
    *reinterpret_cast<int*>(get_buffer())=type_id;
    m_bytes_used+=sizeof(int);
    *reinterpret_cast<Arg*>(get_buffer())=arg;
    m_bytes_used+=sizeof(Arg);
}

//返回的是当前堆区/栈区数组中第m_bytes_used个元素的指针（地址）
char* ZLogLine::get_buffer(){
    return !m_heap_buffer ? &m_stack_buffer[m_bytes_used]:&(m_heap_buffer.get())[m_bytes_used];
}

//输入参数是容量增加的大小
void ZLogLine::resize_buffer(size_t _enlarge){
    //当前栈/堆区buffer还够用，不用扩容
    if(_enlarge==0 || _enlarge+m_bytes_used <= m_cur_buffer_size) return;
    //启用堆区buffer：
    //1. 日志行还没用堆区存储
    if(!m_heap_buffer)
    {
        //如果需要容量小于512，那么就分配512B，以免多次分配；
        size_t heap_s=std::max(m_bytes_used+_enlarge,static_cast<size_t>(512));//注意输入max函数的两个参数类型必须完全一样！
        m_heap_buffer.reset(new char[heap_s]);
        memcpy(m_heap_buffer.get(),m_stack_buffer,m_bytes_used);
        m_cur_buffer_size=heap_s;
    }
    //2. 日志行之前已经用到过堆空间
    else
    {
        //如果扩容需求小于原容量的两倍，把容量置为原容量的2倍
        size_t heap_s=std::max(2*m_cur_buffer_size, m_bytes_used+_enlarge);
        //把内存中的内容从一个指针移到独占指针时要额外小心！
        //以下的处理不会造成多次释放内存
        std::unique_ptr<char[]> new_heap_buffer(new char[heap_s]);
        memcpy(new_heap_buffer.get(),m_heap_buffer.get(),m_bytes_used);
        m_heap_buffer.swap(new_heap_buffer);
        m_cur_buffer_size=heap_s;
    }
}

template<typename Arg>
int ZLogLine::resolve_type(){
    if(typeid(Arg)==typeid(const char*)) return 1;
    if(typeid(Arg)==typeid(char)) return 2;
    if(typeid(Arg)==typeid(long long)) return 3;
    if(typeid(Arg)==typeid(long)) return 4;
    if(typeid(Arg)==typeid(int)) return 5;
    if(typeid(Arg)==typeid(double)) return 6;
    if(typeid(Arg)==typeid(float)) return 7;
    return 0;//注意异常类型处理
}

ZLogLine& ZLogLine::operator<<(std::string _element){
    const char* element=_element.c_str();
    int type_id=resolve_type<const char*>();
    push_to_buffer(element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(const char* _element){
    int type_id=resolve_type<const char*>();
    push_to_buffer(_element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(char _element){
    int type_id=resolve_type<char>();
    push_to_buffer(_element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(long long _element){
    int type_id=resolve_type<long long>();
    push_to_buffer(_element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(long _element){
    int type_id=resolve_type<long>();
    push_to_buffer(_element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(int _element){
    int type_id=resolve_type<int>();
    push_to_buffer(_element, type_id);
    return *this;
}
ZLogLine& ZLogLine::operator<<(double _element){
    int type_id=resolve_type<double>();
    push_to_buffer(_element, type_id);
    return *this;
}

ZLogLine& ZLogLine::operator<<(float _element){
    int type_id=resolve_type<float>();
    push_to_buffer(_element, type_id);
    return *this;
}
