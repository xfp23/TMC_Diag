
# FileLog - C++ 日志类

## 功能特性

- ✅ 自动在exe同级目录创建 `log` 文件夹
- ✅ 以软件打开时间命名日志文件（格式：YYYYMMDD_HHMMSS.txt）
- ✅ 每次运行只创建一个日志文件
- ✅ 支持普通字符串写入
- ✅ 支持CAN报文格式化写入
- ✅ 线程安全（使用mutex保护）
- ✅ 跨平台支持（Windows/Linux）

## 日志格式示例

```
========================================
Log Start: 2024-01-15 14:30:25.123
========================================
[2024-01-15 14:30:25.456] 程序启动成功
[2024-01-15 14:30:25.789] 正在初始化系统...
[2024-01-15 14:30:26.012] CAN : id 123   02-10-01-00-00-00-00-00
[2024-01-15 14:30:26.234] CAN : id 456   FF-AA-55-12-34
[2024-01-15 14:30:26.567] 系统运行正常
[2024-01-15 14:30:26.890] CAN : id 7FF   01-02-03-04-05-06-07-08
[2024-01-15 14:30:27.123] 程序即将退出
```

## 使用方法

### 1. 基本使用

```cpp
#include "FileLog.h"

int main()
{
    // 创建日志对象
    FileLog logger;
    
    // 写入普通日志
    logger.writeLog("这是一条普通日志");
    
    // 写入CAN报文
    unsigned char data[] = {0x02, 0x10, 0x01, 0x00, 0x00};
    logger.writeCanLog(0x123, data, 5);
    
    return 0;
}
```

### 2. API说明

#### 构造函数
```cpp
FileLog();
```
- 自动创建log文件夹（如果不存在）
- 生成以当前时间命名的日志文件

#### writeLog()
```cpp
void writeLog(const std::string& message);
```
- 写入普通字符串日志
- 自动添加时间戳

#### writeCanLog()
```cpp
void writeCanLog(unsigned int id, const unsigned char* data, int dataLen);
```
- 写入CAN报文
- `id`: CAN ID（十六进制显示）
- `data`: 数据字节数组
- `dataLen`: 数据长度
- 格式：`CAN : id XXX   XX-XX-XX-XX...`

#### isOpen()
```cpp
bool isOpen() const;
```
- 检查日志文件是否成功打开

#### flush()
```cpp
void flush();
```
- 手动刷新缓冲区到文件

## 编译说明

### Windows (MSVC)
```bash
cl /EHsc example.cpp FileLog.cpp /Fe:example.exe
```

### Windows (MinGW)
```bash
g++ -std=c++11 example.cpp FileLog.cpp -o example.exe
```

### Linux
```bash
g++ -std=c++11 example.cpp FileLog.cpp -o example -lpthread
```

## 文件结构

```
project/
├── FileLog.h         # 头文件
├── FileLog.cpp       # 实现文件
├── example.cpp       # 使用示例
└── log/              # 自动创建的日志目录
    └── 20240115_143025.txt
```

## 注意事项

1. 日志文件名格式：`YYYYMMDD_HHMMSS.txt`（例：20240115_143025.txt）
2. 如果log文件夹已存在，不会重复创建
3. 每次程序运行创建一个新的日志文件
4. CAN ID以3位十六进制显示，数据字节以2位十六进制显示，用`-`分隔
5. 所有写入操作都是线程安全的
6. 析构函数会自动关闭文件并刷新缓冲区

## 线程安全

所有公共方法都使用mutex保护，可以安全地在多线程环境中使用：

```cpp
#include <thread>

FileLog logger;

void thread1() {
    logger.writeLog("来自线程1的消息");
}

void thread2() {
    unsigned char data[] = {0x01, 0x02};
    logger.writeCanLog(0x100, data, 2);
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);
    t1.join();
    t2.join();
    return 0;
}
```
