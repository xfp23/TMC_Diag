#pragma once

#include <iostream>
#include <thread>
#include "zlgcan.h"
#include <functional>

class UsbCan2EU
{

public:
    enum class Status_t
    {
        OK,
        ERR,
        DEVERR,
        CHERR,
    };

    enum class Channel_t
    {
        CH0,
        CH1,
    };

    enum class BaudRate_t
    {
        Baud_250K,
        Baud_500K,
    };

    typedef struct {
        Channel_t ch;
        uint8_t data[8];
        uint8_t dlc; 
        uint32_t id;
        uint64_t timestamp;
    }CanDataInfo_t;
    typedef struct {
        std::vector<CanDataInfo_t> info;
        uint32_t recv_num;
    }ChannelCanData_t;

    using Callback_Func_t = std::function<void(UsbCan2EU::ChannelCanData_t)>;
    // typedef void(*Recv_Func_t)(ChannelCanData_t CanData);
private:
    const char *baud_str[2] = {"250000","500000"}; // 与枚举值对应，做索引访问
    static constexpr BYTE chn_max = 2;
    // std::thread thd_busload;         // 总线利用率线程
    std::thread thd_handle[chn_max]; // 接收线程
    CHANNEL_HANDLE chn[chn_max] = {};
    DEVICE_HANDLE dev_handle = nullptr;
    void thread_task(CHANNEL_HANDLE chn, int chn_idx);
    std::atomic<bool> g_thd_run[chn_max]; // 原子变量
    // BYTE chanel_data[8];
    Callback_Func_t recv_callback = nullptr;
    Callback_Func_t Trans_callback = nullptr;

    ChannelCanData_t ChCanData = { }; 
public:
    UsbCan2EU(/* args */);
    ~UsbCan2EU();

    Status_t OpenDev();
    Status_t CloseDev();
    /**
     * @brief 开启对应通道
     * 
     * @param channel 通道
     * @param baud 波特率
     * @return Status_t 操作结果
     */
    Status_t OpenChannel(Channel_t channel,BaudRate_t baud);

    /**
     * @brief 关闭通道
     * 
     * @param channel 
     * @return Status_t 
     */
    Status_t CloseChannel(Channel_t channel);
    void RegisterTransmitCallback(Callback_Func_t func); // 发送回调
    void RegisterReceiveCallback(Callback_Func_t func); // 接收回调
    Status_t Transmit(Channel_t ch,uint32_t id,uint8_t *data,uint8_t data_len);
    
};
