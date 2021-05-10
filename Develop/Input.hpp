#pragma once
// #ifndef __INPUT_HPP__
// #define __INPUT_HPP__
#include<iostream>
#include<string>
#include<memory>
#include <chrono>
#include <ctime>
#include <thread>
#include<cstring>
#include<algorithm>
#include<typeinfo>
//***********ZLog************
//为什么非要把ZLog写到这然后还得拆开一部分放到Logger.hpp呢。。
//如果单独把ZLog放到一个文件，会造成头文件环形引用，用预编译指令解决不了，只能把环解开（在其中一个文件里提前声明另一个文件中需要的符号）
//但问题是namespa这种东西不能声明，只能直接定义，所以就只能定义到这了。。ZLog中还有一些其他成员需要依赖Logger中的符号，所以就拆成两部分了。。。。
namespace ZLog{
    //日志等级
    enum LogLevel:char{
    //用char可以省三个字节
    INFO,//信息
    WARN,//警告
    CRIT//错误
    };
}

/*
功能：封装一个日志行结构，将用户输入的参数push进堆区或栈区开辟的buffer
开发者：zkcc
日期：2021.4.26
*/
class ZLogLine{
public:
    ZLogLine()=default;
    ZLogLine(ZLog::LogLevel _level,const char* _filename,const char* _funcname, size_t _line);
    //只有这里声明了需要编译器提供默认操作，才可以；如果没声明，就不允许这种操作了！
    ZLogLine(const ZLogLine&)=default;
    ZLogLine& operator=(const ZLogLine&)=default;
    ZLogLine(ZLogLine&&)=default;
    ZLogLine& operator=(ZLogLine&&)=default;
    ~ZLogLine()=default;//析构不用释放堆区buffer了啊。。堆区buffer是独占指针，自动给你释放好了
    //各输入类型的重载，为了保证实际存入buffer的字节数最少，需要严格控制输入的类型，保证用最小的空间存用户的输入参数
    ZLogLine& operator<<(std::string _element);//C++string不能用，需要转换成const char*
    ZLogLine& operator<<(const char* _element);
    ZLogLine& operator<<(char _element);
    ZLogLine& operator<<(long long _element);
    ZLogLine& operator<<(long _element);
    ZLogLine& operator<<(int _element);
    ZLogLine& operator<<(double _element);
    ZLogLine& operator<<(float _element);
    void pop_to_file(std::ostream& os);//FileWriter调用这个，直接将buffer的内容写入文件中的一行

private:
    //解析重载输入的类型，只是推导一下类型，提供模板参数即可，不必传入参数
    template<typename Arg>
    int resolve_type();
    //采用泛型强制转换技术，可输入绝大多数类型
    //要认识到，存入buffer各地址的是数据
    template<typename Arg>
    void push_to_buffer(Arg arg);
    //为了重载输入参数，解析参数类型的push_to_buffer重载
    //存储时，在数据前面加上1B的type_id，以便在解码时识别类型
    template<typename Arg>
    void push_to_buffer(Arg arg, int type_id);
    char* get_buffer();//获取当前正在使用的buffer指针，如果堆区buffer不为空，返回堆区；否则返回栈区buffer
    void resize_buffer(size_t _enlarge);//push_to_buffer里需要提前检查push后的m_bytes_used超没超栈区buffer的上限，超了需要切换堆区buffer
    uint64_t timestamp_now();//系统获取当前时间
    void format_timestamp(std::ostream & os, uint64_t timestamp);//格式化的时间输出

private:
    std::unique_ptr<char[]> m_heap_buffer;//堆区指针，相当于一个数组指针(堆区是一个指针指向一个数组，栈区就是一个裸数组）
    char m_stack_buffer[256-sizeof(m_heap_buffer)-2*sizeof(size_t)];//栈区buffer，这样设计大小是为了确保ZLogLine类可以256B对齐
    size_t m_bytes_used;//当前用了多少字节，用来判断需不需要换buffer
    size_t m_cur_buffer_size;//当前正在使用的buffer的大小（堆区/栈区）

};

// #endif