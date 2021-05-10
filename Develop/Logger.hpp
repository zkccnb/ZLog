#pragma once
// #ifndef __LOGGER_HPP__
// #define __LOGGER_HPP__
#include"Abstract.hpp"
#include"Output.hpp"

enum state{INIT,READY,SHUT};//全局ZLogger对象当前所处的状态，READY时可以开始向文件写入日志行

class ZLogger{
public:
    ZLogger()=default;
    ZLogger(const char* file_addr, size_t max_bytes);
    ZLogger(const ZLogger& ) = delete;//此类的拷贝构造一定是禁用的！就算是我不写，编译器也会禁用，因为有一个成员是阻止拷贝的！(std::thread)
    ~ZLogger();//将state置为shut
    void push_to_logger(ZLogLine&& _zline);
    void pop_to_writer();
private:
    std::unique_ptr<AbsLog> m_AbsLog;
    std::unique_ptr<FileWriter> m_writer;//所以这个地方可能会引起你的灵魂思考，设计一个类成员变量时，什么时候使用指针，什么时候不使用指针呢
    std::atomic<state> m_state;
    std::thread m_thread;
};



//一个空类，重载+= 调用了ZLogger::push_to_logger，用于把用户传入的日志行送至全局的at_logger对象
class LoggerHelper{
public:
    bool operator+=(ZLogLine& _zline);
};

namespace ZLog{
    #define ZLOG_INFO LoggerHelper()+=ZLogLine(ZLog::LogLevel::INFO,__FILE__,__func__,__LINE__)//记录信息
    #define ZLOG_WARN LoggerHelper()+=ZLogLine(ZLog::LogLevel::WARN,__FILE__,__func__,__LINE__)//记录警告
    #define ZLOG_CRIT LoggerHelper()+=ZLogLine(ZLog::LogLevel::CRIT,__FILE__,__func__,__LINE__)//记录错误
    //初始化日志框架，ZLog只应在主线程中被初始化一次！
    void initialize(const char* file_addr, size_t max_bytes);
}

//#endif