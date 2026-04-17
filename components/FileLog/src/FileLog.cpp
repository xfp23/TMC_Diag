#include "FileLog.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <unistd.h>
    #include <linux/limits.h>
#endif

FileLog::FileLog()
{
    initLogFile();
}

FileLog::~FileLog()
{
    if (logFile.is_open())
    {
        logFile.flush();
        logFile.close();
    }
}

std::string FileLog::getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

std::string FileLog::getExeDirectory()
{
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    size_t pos = exePath.find_last_of("\\/");
    return exePath.substr(0, pos);
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1)
    {
        buffer[len] = '\0';
        std::string exePath(buffer);
        size_t pos = exePath.find_last_of('/');
        return exePath.substr(0, pos);
    }
    return ".";
#endif
}

bool FileLog::createLogDirectory(const std::string& dirPath)
{
    struct stat info;
    
    // 检查目录是否存在
    if (stat(dirPath.c_str(), &info) == 0)
    {
        if (info.st_mode & S_IFDIR)
        {
            return true;  // 目录已存在
        }
    }
    
    // 创建目录
    if (mkdir(dirPath.c_str(), 0755) == 0)
    {
        return true;
    }
    
    return false;
}

bool FileLog::initLogFile()
{
    // 获取exe目录
    std::string exeDir = getExeDirectory();
    
    // 构建log目录路径
    std::string logDir = exeDir + "/log";
    
    // 创建log目录（如果不存在）
    if (!createLogDirectory(logDir))
    {
        return false;
    }
    
    // 获取当前时间作为文件名
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
    
    std::ostringstream filename;
    filename << logDir << "/"
             << std::put_time(&tm_now, "%Y%m%d_%H%M%S")
             << ".txt";
    
    logFilePath = filename.str();
    
    // 打开日志文件
    logFile.open(logFilePath, std::ios::out | std::ios::app);
    
    if (logFile.is_open())
    {
        // 写入启动信息
        logFile << "========================================" << std::endl;
        logFile << "Log Start: " << getCurrentTime() << std::endl;
        logFile << "========================================" << std::endl;
        logFile.flush();
        return true;
    }
    
    return false;
}

void FileLog::writeLog(const std::string& message)
{
    std::lock_guard<std::mutex> lock(logMutex);
    
    if (logFile.is_open())
    {
        logFile << "[" << getCurrentTime() << "] " << message << std::endl;
        logFile.flush();
    }
}

void FileLog::writeCanLog(unsigned int id, const unsigned char* data, int dataLen)
{
    std::lock_guard<std::mutex> lock(logMutex);
    
    if (logFile.is_open())
    {
        std::ostringstream oss;
        oss << "[" << getCurrentTime() << "] CAN : id " 
            << std::setfill('0') << std::setw(3) << std::hex << std::uppercase << id << "   ";
        
        // 写入数据字节（十六进制格式，用-分隔）
        for (int i = 0; i < dataLen; ++i)
        {
            oss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase 
                << static_cast<int>(data[i]);
            if (i < dataLen - 1)
            {
                oss << "-";
            }
        }
        
        logFile << oss.str() << std::endl;
        logFile.flush();
    }
}

void FileLog::flush()
{
    std::lock_guard<std::mutex> lock(logMutex);
    
    if (logFile.is_open())
    {
        logFile.flush();
    }
}

bool FileLog::isOpen() const
{
    return logFile.is_open();
}
