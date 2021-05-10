#pragma once
// #ifndef __OUTPUT_HPP__
// #define __OUTPUT_HPP__
#include"Input.hpp"
#include<fstream>
/*
功能：向实际文件写入一个日志行
开发者：zkcc
日期：2021.4.26
*/
class FileWriter{
public:
    FileWriter(const char* file_addr, size_t max_Mbytes_per_file);
    FileWriter(const FileWriter&)=delete;
    FileWriter(FileWriter&&)=delete;
    ~FileWriter()=default;
    void pop_to_file(ZLogLine&);//向文件写入
private:
    void roll_file();//换下一个文件，关闭当前文件，新建一个文件，打开
private:
    //写入文件的名字
    //示例：/home/zkcc/nanolog/ZLog/Develop/log
    std::string m_file_name;
    size_t m_nfile;//第几个文件的标号
    std::unique_ptr<std::ofstream> m_os;//当前打开的文件（输出流）指针
    size_t m_max_bytes_per_file;//每个文件写入的最大字节数
    size_t m_writen_bytes;//当前写入当前文件的字节
};

//#endif