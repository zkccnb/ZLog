# ZLog
A Simple, Fast Logging Framework Within 500 Lines In C++ For You to Learning And Discussing  
## 文件结构：  
ZLog包含3个文件夹，在正式开始阅读、使用源码前，建议您先浏览**Design**下的日志框架设计思路；  
对于想深入研究并调试ZLog代码的同学，应主要使用**Develop**下的文件，这里包括了我的开发日志（developLog.md）、各模块的单元测试demo（ut_xxx.cpp）、整体性能测试demo（benchmark.cpp）、必要的源文件以及makefile文件；  
对于想直接使用ZLog的需求，可以直接将**Using**中的内容加入您的工程，按照下面的使用说明进行编译、使用即可。  
## A Quick Start：  
* Step 1：初始化ZLog，ZLog在记录日志前必须被初始化，应该在工程的主线程中初始化，且只应被初始化一次：
    demo：
    ```C++
    initialize("/home/zkcc/TestLogFiles/testlog", 10);
    //参数1：准备写入日志文件的地址及文件名，此时写入的文件即为：/home/zkcc/TestLogFiles/testlog1.txt
    //参数2：一个日志文件的大小（单位MB），当记录日志超过此大小则自动更换日志文件。
    ```  
* Step2：初始化后，可以在工程的各个线程记录日志：
ZLog支持3种日志等级：INFO（信息）、WARN（警告）、CRIT（错误）；  
支持8种常见输入类型（及其隐式转换）：int、double、float、long、long long、char、const char*、string。  
demo：
```C++
int i=0;
ZLOG_INFO << "Logging INFO" << i << 0 << 'K' << -42.42;//记录信息
ZLOG_WARN << "Logging WARN" << i << 0 << 'K' << -42.42;//记录警告
ZLOG_CRIT << "Logging CRIT" << i << 0 << 'K' << -42.42;//记录错误
```  
## ZLog特性：  
* 1：在日志框架中，所有日志行对象不存在拷贝
* 2：一行日志对象的大小不超过256B时不开辟堆区空间；如超过256B则采用阶梯扩容机制为日志行开辟堆区空间。
* 3：采用抽象缓冲区队列（AbsLog类）缓存日志，可以确保极端日志记录速度下不会丢失日志；经测试，最多可以保证400个线程同时记录而不会丢失日志。  
* 4：日志的缓存与写入文件分开，采用生产者-消费者模式。消费者（写入文件）在另一个线程独立运行，等待消费者（产生日志的线程）记录日志；这样可以确保更快地缓存日志，避免IO密集的操作频繁阻塞线程。
* 5：可以自定义当一个日志文件超过一定大小后更换一个新的文件继续记录，避免一个日志文件过大导致无法打开的情况。
* 6：支持Linux系统（gcc编译器）、windows系统（clang编译器）。  
## ZLog性能测试：
* 测试环境：Ubantu 19.09 + Intel(R) Core(TM) i5-10210U CPU @ 1.60GHz  + 16.0 GB-RAM 
* 测试结果（benchmark）：
```txt
Thread count: 1
        Average Log Latency = 854 nanoseconds
Thread count: 2
        Average Log Latency = 1324 nanoseconds
        Average Log Latency = 1351 nanoseconds
Thread count: 3
        Average Log Latency = 1750 nanoseconds
        Average Log Latency = 1767 nanoseconds
        Average Log Latency = 1838 nanoseconds
Thread count: 4
        Average Log Latency = 2231 nanoseconds
        Average Log Latency = 2304 nanoseconds
        Average Log Latency = 2232 nanoseconds
        Average Log Latency = 2321 nanoseconds
Thread count: 5
        Average Log Latency = 2729 nanoseconds
        Average Log Latency = 2719 nanoseconds
        Average Log Latency = 2806 nanoseconds
        Average Log Latency = 2844 nanoseconds
        Average Log Latency = 2822 nanoseconds
```  
## Contact Me：
* 我的博客（CSDN）：https://blog.csdn.net/weixin_42923076  
这里有一些写ZLog源码时用到的一些技术细节以及遇到的问题和解决过程。
* 我的邮箱（保证随时可以联系到我！）：zkcc20164020@126.com  
本人水平、精力有限，花在造轮子上的时间实际也不多，所以源码中难免存在一些问题，欢迎前辈和同学与我讨论其中存在的问题，恳请各位大佬批评指正！
