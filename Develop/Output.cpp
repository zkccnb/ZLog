#include"Output.hpp"
FileWriter::FileWriter(const char* file_addr, size_t max_Mbytes_per_file)
:m_file_name(file_addr),m_nfile(0),m_max_bytes_per_file(max_Mbytes_per_file*(1024*1024)),m_writen_bytes(0){
    roll_file();
}

void FileWriter::roll_file(){
    if(m_os)
    {
        m_os->flush();
        m_os->close();
    }
    m_writen_bytes=0;
    m_os.reset(new std::ofstream());
    std::string open_file_name=m_file_name;
    open_file_name.append(std::to_string(++m_nfile));
    open_file_name.append(".txt");//第一个文件的标号是1
    m_os->open(open_file_name, std::ofstream::out | std::ofstream::trunc);//写方式打开一个文件，如果文件存在则将文件原有内容清空
}

void FileWriter::pop_to_file(ZLogLine& _line){
    m_writen_bytes=m_os->tellp();//获取已经记录了多少字节
    if(m_writen_bytes>=m_max_bytes_per_file) roll_file();
    _line.pop_to_file(*m_os);
}

