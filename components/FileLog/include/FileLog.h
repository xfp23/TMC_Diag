#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <vector>

class FileLog
{
private:
    std::ofstream logFile;
    std::string logFilePath;
    std::mutex logMutex;  // 线程安全
    
    // 获取当前时间字符串
    std::string getCurrentTime();
    
    // 获取exe所在目录
    std::string getExeDirectory();
    
    // 创建log目录（如果不存在）
    bool createLogDirectory(const std::string& dirPath);
    
    // 初始化日志文件
    bool initLogFile();

public:
    FileLog();
    ~FileLog();
    
    // 写入普通字符串
    void writeLog(const std::string& message);
    
    // 写入CAN报文
    // id: CAN ID (十六进制)
    // data: CAN数据字节数组
    // dataLen: 数据长度
    void writeCanLog(unsigned int id, const unsigned char* data, int dataLen);
    
    // 刷新日志到文件
    void flush();
    
    // 检查日志是否成功打开
    bool isOpen() const;
};
