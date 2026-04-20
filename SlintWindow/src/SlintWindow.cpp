#include "SlintWindow.h"
#include "FileLog.h"
using namespace std;

SlintWindow::SlintWindow() : ui_(AppWindow::create())
{
    this->BindEvents();
    this->usb_can = new UsbCan2EU();
    this->Gl = new Logic::Global();
    this->log = new FileLog();

    usb_can->RegisterReceiveCallback([this](auto data)
                                     { this->CanReceiveLoop(data); });
    Gl->Can_RegisterCanTrans(
        [this](uint32_t id, uint8_t *data, uint16_t len)
        {
            std::stringstream ss;

            ss << "TX ID : 0x" << std::hex << std::uppercase << id << " DATA: [";

            for(int i = 0; i < len; i++)
            {
                ss << std::setfill('0') << std::setw(2) << std::hex << (int)data[i] << " ";
            }

            ss << ']';
            
            this->append_log(ss.str());
            if(this->log->isOpen())
            {
                this->log->writeLog(ss.str());
            }
            return this->usb_can->Transmit(
                       static_cast<UsbCan2EU::Channel_t>(this->ch),
                        // UsbCan2EU::Channel_t::CH0,
                       id,
                       data,
                       len) == UsbCan2EU::Status_t::OK;
        });
}

SlintWindow::~SlintWindow()
{

    delete this->usb_can;
    // delete this->global;
}

// Private:
void SlintWindow::BindEvents()
{
    // auto ui = this->ui_;

    ui_->on_connect_clicked([this]
                            { BtnClick_ConnectDevice(); });

    ui_->on_disconnect_clicked([this]
                               { BtnClick_DisConnectDevice(); });

    ui_->on_start_test_clicked([this]
                               { BtnClick_StartDiag(); });

    // ui_->window().on_close_requested([this](){WindowClick_Close();});
    ui_->window().on_close_requested([this]()
                                     {
        
        WindowClick_Close();
        // 返回 CloseRequestResponse::HideWindow 以允许关闭（隐藏）
        // 返回 CloseRequestResponse::KeepWindow 可拦截并阻止关闭
        return slint::CloseRequestResponse::HideWindow; });
}

void SlintWindow::show()
{
    this->ui_->run();
}
void SlintWindow::BtnClick_ConnectDevice()
{
    if (usb_can->OpenDev() != UsbCan2EU::Status_t::OK)
    {
        this->ui_->set_OpenDevError(true);
        return;
    }
    
    int ch = ui_->get_selected_can_channel();
    int baud = ui_->get_selected_can_baud();
    this->ch = ch;
    // if (usb_can->OpenChannel((UsbCan2EU::Channel_t)ch, (UsbCan2EU::BaudRate_t)baud) != UsbCan2EU::Status_t::OK)
    // {
    //     this->ui_->set_OpenDevError(true);
    //     return;
    // }
    usb_can->OpenChannel(UsbCan2EU::Channel_t::CH0, UsbCan2EU::BaudRate_t::Baud_500K);
    usb_can->OpenChannel(UsbCan2EU::Channel_t::CH1, UsbCan2EU::BaudRate_t::Baud_500K);
    this->ui_->set_OpenDevBtnEnable(false);
    this->ui_->set_CloseDevBtnEnable(true);
    this->ui_->set_StartDiagBtnEnable(true);
}

void SlintWindow::BtnClick_DisConnectDevice()
{
    // usb_can->CloseDev();
    this->ch = ui_->get_selected_can_channel();
    usb_can->CloseChannel(UsbCan2EU::Channel_t::CH0);
    usb_can->CloseChannel(UsbCan2EU::Channel_t::CH1);
    this->ui_->set_OpenDevBtnEnable(true);
    this->ui_->set_CloseDevBtnEnable(false);
    this->ui_->set_StartDiagBtnEnable(false);
}

// void SlintWindow::BtnClick_StartDiag()
// {
//     this->ui_->set_StartDiagBtnEnable(false);
//     this->ui_->set_test_status("正在测试");
//     this->ui_->set_diag_status(1);
//     // this->ui_->set_step1_color(slint::Color::from_argb_uint8(255, 255, 0, 0)); // 红色
//     this->ui_->set_step_one_status(0);
//     this->ui_->set_step_two_status(0);
//     this->ui_->set_step_three_status(0);
//     this->ui_->set_step_four_status(0);

//     std::thread([this]() { // 开个线程跑后台
//         if (!this->Gl->Step1_SafeUnlock())
//         {
//             slint::invoke_from_event_loop([this]() { // 在UI线程更新UI
//                 this->ui_->set_StartDiagBtnEnable(true);
//                 this->ui_->set_test_status("失败-测试不通过");
//                 this->ui_->set_diag_status(3);
//                 this->ui_->set_StartDiagBtnEnable(true);
//                 this->ui_->set_step_one_status(2);
//             });

//             return;
//         }

//             slint::invoke_from_event_loop([this]() { // 在UI线程更新UI
//                 this->ui_->set_step_one_status(1);
//             });

//         if (!this->Gl->Step2_WriteF1C1())
//         {
//             slint::invoke_from_event_loop([this]() { // 在UI线程更新UI
//                 this->ui_->set_StartDiagBtnEnable(true);
//                 this->ui_->set_test_status("失败-测试不通过");
//                 this->ui_->set_diag_status(3);
//                 this->ui_->set_StartDiagBtnEnable(true);
//                 this->ui_->set_step_two_status(2);
//             });

//             return;
//         }

//             slint::invoke_from_event_loop([this]() { // 在UI线程更新UI
//                 this->ui_->set_step_two_status(1);
//             });

//         if (!this->Gl->Step3_ReadF1C1())
//         {
//             slint::invoke_from_event_loop([this]() { // 在UI线程更新UI
//                 this->ui_->set_StartDiagBtnEnable(true);
//                 this->ui_->set_test_status("失败-测试不通过");
//                 this->ui_->set_diag_status(3);
//                 this->ui_->set_StartDiagBtnEnable(true);
//                 this->ui_->set_step_three_status(2);
//             });
//         }

//             slint::invoke_from_event_loop([this]() { // 在UI线程更新UI
//                 this->ui_->set_step_three_status(1);
//             });

//         if (!this->Gl->Step4_ReadF180())
//         {
//             slint::invoke_from_event_loop([this]() { // 在UI线程更新UI
//                 this->ui_->set_StartDiagBtnEnable(true);
//                 this->ui_->set_test_status("失败-测试不通过");
//                 this->ui_->set_diag_status(3);
//                 this->ui_->set_StartDiagBtnEnable(true);
//                 this->ui_->set_step_four_status(2);
//             });

//             return;
//         }

//             slint::invoke_from_event_loop([this]() { // 在UI线程更新UI
//                 this->ui_->set_step_four_status(1);
//             });

//     }).detach();

//     this->ui_->set_StartDiagBtnEnable(true);
//     this->ui_->set_test_status("测试通过");
//     this->ui_->set_diag_status(1);
//     this->ui_->set_StartDiagBtnEnable(true);
// }

void SlintWindow::BtnClick_StartDiag()
{
    // 1. 初始化界面状态 (UI 线程)
    this->ui_->set_StartDiagBtnEnable(false);
    this->ui_->set_test_status("正在测试...");
    this->ui_->set_diag_status(1); // 运行中颜色
    this->ui_->set_step_one_status(0);
    this->ui_->set_step_two_status(0);
    this->ui_->set_step_three_status(0);
    this->ui_->set_step_four_status(0);
    this->ui_->set_log_content("");

    // 2. 开启后台业务线程
    std::thread([this]()
                {
                    // --- 步骤 1 ---
                    if (!this->Gl->Step1_SafeUnlock())
                    {
                        this->update_ui_on_failure(1); // 封装一个失败处理函数
                        return;
                    }
                    slint::invoke_from_event_loop([this]()
                                                  { this->ui_->set_step_one_status(1); });

                    // --- 步骤 2 ---
                    if (!this->Gl->Step2_WriteF1C1())
                    {
                        this->update_ui_on_failure(2);
                        return;
                    }
                    slint::invoke_from_event_loop([this]()
                                                  { this->ui_->set_step_two_status(1); });

                    // --- 步骤 3 ---
                    if (!this->Gl->Step3_ReadF1C1())
                    {
                        this->update_ui_on_failure(3);
                        return;
                    }
                    slint::invoke_from_event_loop([this]()
                                                  { this->ui_->set_step_three_status(1); });

                    // --- 步骤 4 ---
                    if (!this->Gl->Step4_ReadF180())
                    {
                        this->update_ui_on_failure(4);
                        return;
                    }
                    slint::invoke_from_event_loop([this]()
                                                  { this->ui_->set_step_four_status(1); });

                    // --- 只有全部跑完才会执行到这里：显示成功 ---
                    slint::invoke_from_event_loop([this]()
                                                  {
            this->ui_->set_test_status("测试通过");
            this->ui_->set_diag_status(2); // 绿色
            this->ui_->set_StartDiagBtnEnable(true); });
                })
        .detach();

    // 注意：函数末尾这里不要再写任何 set_test_status 了！
}

void SlintWindow::update_ui_on_failure(int step_index)
{
    slint::invoke_from_event_loop([this, step_index]()
                                  {
        this->ui_->set_test_status("失败-测试不通过");
        this->ui_->set_diag_status(3); // 红色
        this->ui_->set_StartDiagBtnEnable(true);
        
        // 根据哪一步失败，设置对应的进度图标
        if (step_index == 1) this->ui_->set_step_one_status(2);
        else if (step_index == 2) this->ui_->set_step_two_status(2);
        else if (step_index == 3) this->ui_->set_step_three_status(2);
        else if (step_index == 4) this->ui_->set_step_four_status(2); });
}

void SlintWindow::WindowClick_Close()
{
    usb_can->CloseDev();
}

void SlintWindow::CanReceiveLoop(UsbCan2EU::ChannelCanData_t data)
{
    for (const auto &frame : data.info)
    {
        this->Gl->Can_ReviveCallback(frame.id, (uint8_t *)frame.data, frame.dlc);

        // if (this->log->isOpen())
        // {
        //     this->log->writeCanLog(frame.id, frame.data, frame.dlc);
        // }

        if (frame.id == 0x18DAF119)
        {
            std::stringstream ss;

            ss << "RX ID : 0x" << std::hex << std::uppercase << frame.id << "   DATA: [";

            for (int i = 0; i < frame.dlc; i++)
            {
                ss << std::setfill('0') << std::setw(2) << std::hex << (int)frame.data[i] << " ";
            }

            ss << ']';

            this->append_log(ss.str());
            if (this->log->isOpen())
            {
                this->log->writeLog(ss.str());
            }
        }
    }
}

void SlintWindow::append_log(std::string message)
{
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buf[20];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now));

    std::string formatted_msg = "[" + std::string(buf) + "] " + message + "\n";

    // 切换到 UI 线程更新
    slint::invoke_from_event_loop([this, formatted_msg]()
                                  {
        // 先获取旧内容，再追加新内容
        auto old_text = this->ui_->get_log_content();
        this->ui_->set_log_content(old_text + slint::SharedString(formatted_msg)); });
}