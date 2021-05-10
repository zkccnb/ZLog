# 开发日志  
## 输入-缓存模块：  
* 2021.4.28：  
**开发内容**：默认日志存入buffer、堆区/栈区buffer切换、日志行写入文件函数  
**测试内容**：默认日志存入buffer、堆区/栈区buffer切换、默认日志内容从buffer输出到标准输出  
**测试结果**：用多个测例测试，堆区/栈区切换正常，默认日志存入、输出到标准输出正常。  
* 2021.4.29：  
**开发内容**：用户输入参数类型推导，存入日志行buffer  
**测试内容1**：用户输入参数测试  
测例：
```C++
test_line<<c<<s<<"xxxx"<<'c'<<(long long)(42614627147291)<<42614627147291<<6.824<<6.824f<<32;
test_line<< "Logging " << benchmark << i << 0 << 'K' << -42.42<<'\n';  
```  
**测试结果1**：均可正常存入，未见类型推导异常及存入异常。  
**测试内容2**：10w行用户输入参数+默认参数日志单线程写入硬盘文件速度。  
测例：
```C++
test_line<< "Logging " << benchmark << i << 0 << 'K' << -42.42<<'\n';  
```  
**测试结果2**：10w行日志单线程写入文件耗时：**267.812ms**。  
## 输出-文件模块：  
* 2021.4.30：  
**开发内容**：单线程将日志行的内存缓存写入文件，按用户指定单个文件最大大小进行切换文件。  
**测试内容**：10w行日志（10+MB）存入文件，观察是否按指定最大文件大小切换文件。  
测例：
```C++
test_line<<c<<s<<"xxxx"<<'c'<<(long long)(42614627147291)<<42614627147291<<6.824<<6.824f<<32;
```  
**测试结果**：10w行日志单线程写入文件正常，切换文件正常。  
## 抽象文件模块：  
* 2021.5.1~5.3：  
**开发内容**： AbsFile类、AbsLog类，有保证的抽象日志记录缓存体系。  
* 2021.5.6：  
**测试内容1**：单线程记录4w条日志，观察absfile的切换是否会导致丢日志。  
测例：
```C++
//单线程记录日志
void single_th(){
    AbsLog test_abs;
    FileWriter fw("/home/zkcc/nanolog/ZLog/Develop/TestLogFiles/testlog",20);
    for(int i=0;i<40000;i++){
        test_abs.push_to_abslog(std::move(ZLogLine(ZLog::LogLevel::INFO,__FILE__,__func__,__LINE__)<<" +test "<<i));
    }
    for(int i=0;i<40000;i++){
        ZLogLine _zline;
        if(test_abs.pop_to_logger(_zline))
            fw.pop_to_file(_zline);
    }
}
```  
**测试结果1**：经过修改后，单线程push日志正常，切换absfile正常，不会造成丢日志。   
**测试内容2**：400个线程共记录16w行日志，观察多线程下absfile切换是否丢日志。  
测例：
```C++
//多线程记录日志
AbsLog test_abs;
void func(){
    for(int i=0;i<400;i++){
        test_abs.push_to_abslog(std::move(ZLogLine(ZLog::LogLevel::INFO,__FILE__,__func__,__LINE__)<<" +test "<<i));
    }
}
void multi_th(){
    vector<thread> ths;
    for(int i=0;i<400;i++){
        ths.emplace_back(func);
    }
    for(int i=0;i<400;i++){
        ths[i].join();
    }
    FileWriter fw("/home/zkcc/nanolog/ZLog/Develop/TestLogFiles/testlog",20);
    for(int i=0;i<1000000;i++){
        ZLogLine _zline;
        if(test_abs.pop_to_logger(_zline))
            fw.pop_to_file(_zline);
    }
}
```  
**测试结果2**： 经过修改后，多线程push日志正常，切换absfile正常，400个线程同时高强度记录日志不会造成任何日志丢失。  
## 交换模块：  
* 2021.5.4~5.5：  
**开发内容**： ZLogger类（连接输出模块和抽象文件模块）、ZLog对外接口等内容。  
* 2021.5.7：  
**测试内容**：和抽象模块类似。  
## 整体测试（benchmark）：  
* 2021.5.7：  
**run_benchmark函数**：  
用f函数循环创建thread_count个线程，然后join各线程，等待其结束。  
**nanolog_benchmark函数**：  
每个线程执行的函数，循环10w次，每次循环向指定路径文本文件中输出一行日志，  
同时记录输出日志花费的时间，打印到控制台。  
**timestamp_now函数**：  
以纳秒为单位记录当前程序段执行的时间。  
**main函数**：  
先初始化日志系统，设定输出日志文件路径，每个日志文件的最大大小等；  
分别测试同时运行线程数1~5的日志耗时。  
**测试结果**：  
```txt
Thread count: 1
        Average NanoLog Latency = 1483 nanoseconds
Thread count: 2
        Average NanoLog Latency = 1967 nanoseconds
        Average NanoLog Latency = 2063 nanoseconds
Thread count: 3
        Average NanoLog Latency = 3042 nanoseconds
        Average NanoLog Latency = 3120 nanoseconds
        Average NanoLog Latency = 3157 nanoseconds
Thread count: 4
        Average NanoLog Latency = 4067 nanoseconds
        Average NanoLog Latency = 4203 nanoseconds
        Average NanoLog Latency = 4207 nanoseconds
        Average NanoLog Latency = 4250 nanoseconds
Thread count: 5
        Average NanoLog Latency = 5704 nanoseconds
        Average NanoLog Latency = 5861 nanoseconds
        Average NanoLog Latency = 5863 nanoseconds
        Average NanoLog Latency = 5861 nanoseconds
        Average NanoLog Latency = 5869 nanoseconds
```  
没有任何日志行丢失