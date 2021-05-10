#include"Input.hpp"
#include<fstream>
using namespace std;
void xx(std::ofstream& os){
    ZLogLine test_line(ZLog::LogLevel::INFO,__FILE__,__func__,__LINE__);
    string s="s_xxxx";
    char c[]="xxxx";
    char const * const benchmark = "benchmark";
    int i=0;
    test_line<<c<<s<<"xxxx"<<'c'<<(long long)(42614627147291)<<42614627147291<<6.824<<6.824f<<32;
    test_line<< "Logging " << benchmark << i << 0 << 'K' << -42.42<<'\n';
    test_line.pop_to_file(os);
}
int main(){
    std::ofstream* os=new std::ofstream();
    std::string log_file_name = "/home/zkcc/nanolog/ZLog/Develop/testlog.txt";
    os->open(log_file_name, std::ofstream::out | std::ofstream::trunc);
    for(int i=0;i<100000;i++) xx(*os);
    os->flush();
    os->close();
}
