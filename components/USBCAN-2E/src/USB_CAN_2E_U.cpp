// #include <windows.h>

#include "USB_CAN_2E_U.h"
// #include "stdafx.h"
#include "string.h"

// CRITICAL_SECTION print_mutex;

// #define LOCKED_BLOCK(code) do { \
//     EnterCriticalSection(&print_mutex); \
//     code; \
//     LeaveCriticalSection(&print_mutex); \
// } while(0)

UsbCan2EU::UsbCan2EU()
{

}

UsbCan2EU::~UsbCan2EU()
{

}

UsbCan2EU::Status_t UsbCan2EU::OpenDev()
{
    this->dev_handle = ZCAN_OpenDevice(21,0,0);

    if(this->dev_handle == nullptr)
    {
        return Status_t::ERR;
    }

    return Status_t::OK;
}

UsbCan2EU::Status_t UsbCan2EU::CloseDev()
{
    if (this->dev_handle == nullptr)
    {
        return Status_t::DEVERR;
    }

    // if (this->chn[(int)channel] == nullptr)
    // {
    //     return Status_t::CHERR;
    // }
    if (0 == ZCAN_CloseDevice(this->dev_handle))
    {
        return Status_t::ERR;
    }

    return Status_t::OK;
}

UsbCan2EU::Status_t UsbCan2EU::OpenChannel(Channel_t channel,BaudRate_t baud)
{
    char path[24] = {};

    sprintf_s(path,24,"%d/baud_rate",(int)channel);

    if(ZCAN_SetValue(this->dev_handle, path, this->baud_str[(int)baud]) == 0)
    {
        return Status_t::ERR;
    }

	ZCAN_CHANNEL_INIT_CONFIG config;	// 通道结构体
	memset(&config, 0, sizeof(config));
	config.can_type = 0;				// 0 = CAN，CAN设备必须是初始化CAN!!
	config.can.mode = 0;				// 0-正常模式，1-只听模式
	//config.can.filter = 1;				//开启滤波
	config.can.acc_code = 0;			// 验收码（USBCAN-8E-U 必须设置验收码和屏蔽码，本系列其他型号可忽略）
	config.can.acc_mask = 0xffffffff;	// 屏蔽码

    this->chn[(int)channel] = ZCAN_InitCAN(this->dev_handle, (int)channel, &config);

    if(this->chn[(int)channel] == nullptr)
    {
        return Status_t::ERR;
    }

    if (ZCAN_StartCAN(this->chn[(int)channel]) == STATUS_ERR) {
        return Status_t::ERR;
    }

    this->g_thd_run[(int)channel] = true; // 给每个通道一个独立的运行标志
    this->thd_handle[(int)channel] = std::thread(&UsbCan2EU::thread_task, this, this->chn[(int)channel], (int)channel);
    return Status_t::OK;
}

void UsbCan2EU::thread_task(CHANNEL_HANDLE chn, int chn_idx) 
{
    ZCAN_Receive_Data canData[100];
    
    while (this->g_thd_run[chn_idx]) 
    {
        uint32_t num = ZCAN_GetReceiveNum(chn, 0);
        if (num > 0) 
        {
            uint32_t ReceiveNum = ZCAN_Receive(chn, canData, 100, 50);
            if (ReceiveNum > 0 && this->recv_callback != nullptr) 
            {
                
                ChannelCanData_t localData; 
                localData.recv_num = ReceiveNum;
                localData.info.resize(ReceiveNum);

                for (uint32_t i = 0; i < ReceiveNum; i++) {
                    localData.info[i].ch = (Channel_t)chn_idx;
                    localData.info[i].id = GET_ID(canData[i].frame.can_id);
                    localData.info[i].dlc = canData[i].frame.can_dlc;
                    memcpy(localData.info[i].data, canData[i].frame.data, 8);
                }

                if(this->recv_callback != nullptr)
                {
                this->recv_callback(localData);
                }

            }
        }
    if (num == 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


void UsbCan2EU::RegisterTransmitCallback(Callback_Func_t func)
{
    this->Trans_callback = func;
}

void UsbCan2EU::RegisterReceiveCallback(Callback_Func_t func)
{
    this->recv_callback = func;
}

UsbCan2EU::Status_t UsbCan2EU::Transmit(Channel_t ch, uint32_t id,uint8_t *data,uint8_t data_len)
{
    if(this->dev_handle == nullptr)
    {
        return Status_t::DEVERR;
    }

    if(this->chn[(int)ch] == nullptr)
    {
        return Status_t::CHERR;
    }

    ZCAN_Transmit_Data canData = {};
    memset(&canData, 0, sizeof(canData));
    canData.frame.can_id = MAKE_CAN_ID(id,0,0,0);
    canData.frame.can_dlc = 8;
    canData.transmit_type = 0;

    memcpy((void*)canData.frame.data,data,8);
    ZCAN_Transmit(this->chn[(int)ch],&canData,1);

    return Status_t::OK;
}

UsbCan2EU::Status_t UsbCan2EU::CloseChannel(Channel_t channel) {

    if(this->dev_handle == nullptr)
    {
        return Status_t::DEVERR;
    }

    if(this->chn[(int)channel] == nullptr)
    {
        return Status_t::CHERR;
    }

    int ch = (int)channel;

    this->g_thd_run[ch] = false;

    if (this->thd_handle[ch].joinable()) {
        this->thd_handle[ch].join();
    }

    if (this->chn[ch] != nullptr) {


        if(ZCAN_ResetCAN(this->chn[ch]) == 0)
        {
            return Status_t::ERR;
        }
        this->chn[ch] = nullptr;
    }

    return Status_t::OK;
}

