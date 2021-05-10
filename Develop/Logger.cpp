#include"Logger.hpp"

ZLogger::ZLogger(const char* file_addr, size_t max_bytes):
m_state(state::INIT),
m_AbsLog(new AbsLog()),
m_writer(new FileWriter(file_addr,max_bytes)),
m_thread(&ZLogger::pop_to_writer,this)
{
    m_state.store(state::READY);
}

ZLogger::~ZLogger(){
    m_state.store(state::SHUT);
    m_thread.join();
}

void ZLogger::push_to_logger(ZLogLine&& _zline){
    m_AbsLog->push_to_abslog(std::move(_zline));
}

void ZLogger::pop_to_writer(){
    ZLogLine zline;
    //用户进程没结束前，一直轮询等待logger状态准备好、日志过来
    while(m_state.load() != state::SHUT){
        if(m_state.load()==state::READY && m_AbsLog->pop_to_logger(zline))
            m_writer->pop_to_file(zline);
        //如果logger没有准备好或当前没有日志过来，则此线程挂起50ms，因为很可能一时半会儿也没有日志，在此轮询很吃性能，且睡眠也不会丢失任何日志
        else std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
    //用户的进程结束后，此线程需要继续保留一小会儿，确保shut前没来得及记录的日志都记录上
    while(m_AbsLog->pop_to_logger(zline))
        m_writer->pop_to_file(zline);
}
//编译器会将“类、内联函数、const变量”视作文件私有；所以，原则上只有这三种东西可以在头文件中定义，其他东西应放在源文件种定义（否则多个源文件引用一个头文件会造成重定义错误）
//如果把下面的东西放在头文件，则会出现main.cpp和Logger.cpp两个源文件同时引用了Logger.hpp头文件，造成符号重定义链接错误
std::atomic<ZLogger*> at_logger;//多线程下唯一的全局变量
std::unique_ptr<ZLogger> logger;//很细节！由于ZLogger不允许拷贝，所以只能用一个独占指针先开辟一块区域，再把这块区域的控制权交给上面的at_logger裸指针，这样就避免了内存的拷贝，完成了初始化

bool LoggerHelper::operator+=(ZLogLine& _zline){
        at_logger.load()->push_to_logger(std::move(_zline));
        return true;
}

void ZLog::initialize(const char* file_addr, size_t max_bytes){
    logger.reset(new ZLogger(file_addr,max_bytes));
    at_logger.store(logger.get());
}