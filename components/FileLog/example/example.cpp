#include "FileLog.h"
#include <iostream>

int main()
{
    // 创建日志对象（自动在exe同级目录下创建log文件夹和日志文件）
    FileLog logger;
    
    // 检查日志是否成功打开
    if (!logger.isOpen())
    {
        std::cerr << "Failed to open log file!" << std::endl;
        return 1;
    }
    
    // 写入普通字符串日志
    logger.writeLog("程序启动成功");
    logger.writeLog("正在初始化系统...");
    
    // 模拟CAN报文数据
    unsigned char canData1[] = {0x02, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    logger.writeCanLog(0x123, canData1, 8);
    
    unsigned char canData2[] = {0xFF, 0xAA, 0x55, 0x12, 0x34};
    logger.writeCanLog(0x456, canData2, 5);
    
    // 继续写入普通日志
    logger.writeLog("系统运行正常");
    
    // CAN报文示例
    unsigned char canData3[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    logger.writeCanLog(0x7FF, canData3, 8);
    
    logger.writeLog("程序即将退出");
    
    // 析构函数会自动关闭文件
    return 0;
}
