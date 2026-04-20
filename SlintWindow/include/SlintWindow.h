/**
 * @file SlintWindow.h
 * @author xfp23
 * @brief 
 * @version 0.1
 * @date 2026-04-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once
#include "Global.h"
// using namespace Logic;
#include "main.h"
#include "USB_CAN_2E_U.h"

class SlintWindow
{
private:
    slint::ComponentHandle<AppWindow> ui_;
    
    UsbCan2EU *usb_can = nullptr;
    Logic::Global *Gl = nullptr;
    FileLog *log = nullptr;

    int ch = 0;

    void WindowClick_Close();

    void append_log(std::string message);

    void BindEvents();
    void BtnClick_ConnectDevice();
    void BtnClick_DisConnectDevice();
    void BtnClick_StartDiag();
    void update_ui_on_failure(int step_index);
    void CanReceiveLoop(UsbCan2EU::ChannelCanData_t data);
public:
    SlintWindow();
    ~SlintWindow();

    void show();
};
