#include"Output.hpp"
using namespace std;
void xx(){
    ZLogLine test_line(ZLog::LogLevel::INFO,__FILE__,__func__,__LINE__);
    string s="s_xxxx";
    char c[]="xxxx";
    char const * const benchmark = "benchmark";
    int i=0;
    test_line<<c<<s<<"xxxx"<<'c'<<(long long)(42614627147291)<<42614627147291<<6.824<<6.824f<<32;
    //test_line<< "Logging " << benchmark << i << 0 << 'K' << -42.42<<'\n';
    FileWriter fw("/home/zkcc/nanolog/ZLog/Develop/TestLogFiles/testlog",1);//每个文件最多10MB
    for(int i=0;i<100000;i++) fw.pop_to_file(test_line);
}
int main(){
    xx();
}
