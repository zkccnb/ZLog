#include"Abstract.hpp"
#include"Output.hpp"
using namespace std;
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
int main(){
    single_th();//单线程，切换absfile正常，没有丢失日志
    multi_th();//解决多线程隐患后，400个线程共记录16w条日志切换absfile不会造成丢失日志
}