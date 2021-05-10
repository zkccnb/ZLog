#include"Logger.hpp"
using namespace std;
using namespace ZLog;
//单线程记录日志
void single_th(){
    for(int i=0;i<200000;i++){
        ZLOG_INFO<<" test + "<<i;
    }
}
//多线程记录日志
AbsLog test_abs;
void func(){
    for(int i=0;i<10000;i++){
        ZLOG_INFO<<" test + "<<i;
    }
}
void multi_th(){
    vector<thread> ths;
    for(int i=0;i<20;i++){
        ths.emplace_back(func);
    }
    for(int i=0;i<20;i++){
        ths[i].join();
    }
}
int main(){
    initialize("/home/zkcc/nanolog/ZLog/Develop/TestLogFiles/testlog",10);
    //single_th();//单线程，切换absfile正常，没有丢失日志
    multi_th();//解决多线程隐患后，400个线程共记录16w条日志切换absfile不会造成丢失日志
}