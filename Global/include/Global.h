#pragma once

#include "main.h"
#include "USB_CAN_2E_U.h"
// #include "SlintWindow.h"
#include "FileLog.h"


namespace Logic
{
    class Global
    {

    public:
       using Can_TransmitFunc_t = std::function<bool(uint32_t id, uint8_t *data, uint16_t len)>;
        // typedef bool(*Can_ReceiveCallback_t(uint32_t id, uint8_t *data, uint16_t len));

    private:

    typedef struct 
    {
        uint32_t recv_id; // 要接收的id
        uint8_t data[256]; // 要接收的databuffer
        size_t len; // 要接收的字节长度
        bool complete;
        bool en; // 接收使能
        uint16_t received_size;
    }RecevBuck_t;

        const uint32_t RelayId = 0x10000000;
        const uint32_t DevPhySicalId = 0x18DA19F1;
        const uint32_t DevResponseId = 0x18DAF119;
        Can_TransmitFunc_t Can_TransmitPtr = nullptr;
        const uint8_t xorArray[4] = {0x54, 0x4D, 0x43, 0xCC};
        bool Transmit(uint32_t id,uint8_t *data,uint16_t len);
        RecevBuck_t buck;
        // Can_ReceiveCallback_t RecvCallback;

        /**
         * @brief can接收
         * 
         * @param id 帧id
         * @param data 数据
         * @param len 接收长度
         * @param timeout 超时时间
         * @return true 成功
         * @return false 失败
         */
        bool Can_Receive(uint32_t id, uint8_t *data, uint16_t len, int timeout);


        bool OpenRelay(); // 打开继电器
        bool CloseRelay(); // 关闭继电器
        void delay(uint32_t ms);

        void CalculateKeyL1(uint8_t *seed,uint8_t *out);

    public:
        Global();
        ~Global();

        void Can_RegisterCanTrans(Can_TransmitFunc_t);
        void Can_ReviveCallback(uint32_t id, uint8_t *data, uint16_t len);



        bool Step1_SafeUnlock(); 
        bool Step2_WriteF1C1();
        bool Step3_ReadF1C1(); // 这里读取是要重新上下电
        bool Step4_ReadF180();
   
    };

}
