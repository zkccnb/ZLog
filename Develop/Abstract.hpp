#pragma once
// #ifndef __ABSTRACT_HPP__
// #define __ABSTRACT_HPP__
#include<atomic>
#include<queue>
#include"Input.hpp"

/*
功能：自旋锁，切换抽象日志文件时需要自旋锁住
开发者：zkcc
日期：2021.5.1
*/
class spinLock{
public:
    spinLock(std::atomic_flag& atf):m_atf(atf){
        while(m_atf.test_and_set(std::memory_order_acquire)) ;//内存序选择为：阻止后面的语句到此句前执行
    }
    ~spinLock(){
        m_atf.clear(std::memory_order_release);//内存序选择为：阻止前面的语句到此句后执行
    }
private:
    std::atomic_flag& m_atf;
};

/*
功能：封装一个ZLogLine日志行对象，确保此类对象256B对齐
开发者：zkcc
日期：2021.5.1
*/
struct Line
{
    Line(ZLogLine&& _zline):m_line(std::move(_zline)){}
    ~Line()=default;
    ZLogLine m_line;
private:
    char align[256-sizeof(ZLogLine)];//保证此类对象256B对齐
};

/*
功能：一个抽象的日志文件，封装了一个日志行数组，支持对其中某一行的日志进行pop/push
开发者：zkcc
日期：2021.5.1
*/
class AbsFile{
public:
    AbsFile();
    ~AbsFile();
    //阻止拷贝
    AbsFile(const AbsFile&)=delete;
    AbsFile& operator=(const AbsFile&)=delete;
    //AbsLog对象会调用此函数将一个日志行存入到当前的AbsFile中
    bool push_to_absfile(ZLogLine&& _zline,size_t cur_line);
    //AbsLog会调用此函数，将某一行的日志行送出去，最终写入文件
    bool pop_to_abslog(ZLogLine& _zline, size_t cur_line);
public:
    static const size_t m_max_line=32768;//一个抽象文件的最大行数
private:
    Line* m_lines;//日志行指针，指向当前抽象文件中的所有日志行
    //注意一个细节，必须用静态成员变量类内初始化其他成员变量（非静态变量在实例化前还没被分配内存呢。。）
    std::atomic<bool> m_isWriten[m_max_line];//标记当前行是否已经存入日志行了？
};

/*
功能：一个抽象的日志文件，封装了一个日志行数组，支持对其中某一行的日志进行pop/push
开发者：zkcc
日期：2021.5.2
*/
class AbsLog{
public:
    AbsLog();
    ~AbsLog()=default;
    AbsLog(const AbsLog&)=delete;
    //把_zline存到m_cur_pushfile的m_cur_pushline行
    void push_to_abslog(ZLogLine&& _zline);
    //通过引用把m_cur_popfile的m_pop_line行的日志行通过引用参数送至ZLogger对象中
    bool pop_to_logger(ZLogLine& _zline);
private:
    //当队列中最后一个absfile也装满数据，需要new一个AbsFile对象push入队，m_cur_pushfile置为此元素
    void set_new_pushfile();
    //当前的AbsFile已读完，把此AbsFile pop出队，m_cur_popfile置为队首元素（AbsFile中的push_to_absfile方法会通知）
    void get_next_popfile();
private:
    std::queue<std::unique_ptr<AbsFile>> m_fileq;//AbsFile的队列，保证日志记录和写入文件全过程不会丢失日志，多线程同时读此队列时需加锁
    //下面两个指针没必要用独占指针啊，因为他俩总是队列中的独占指针get到的内容，队列中的独占指针会确保内存的正确释放
    //如果用独占指针反而会出错，因为两个独占指针不能指向同一片区域！
    //为什么pop相关的AbsFile和cur_line不用原子量呢，是因为写入文件的工作始终只由一个线程完成。。不需要同步
    std::atomic<AbsFile*> m_cur_pushfile;//当前正在记入日志的AbsFile
    AbsFile* m_cur_popfile;//当前正在写入文件的AbsFile
    std::atomic<size_t> m_cur_pushline;//当前push到AbsFile的行数
    size_t m_cur_popline;//当前pop到文件的行数
    std::atomic_flag m_spin;//用于创造一个自旋锁，当需要往队列中入队出队时需锁住队列
};

//#endif