#include "app-window.h"
#include <iostream>

int main() {
    auto ui = AppWindow::create();

    ui->on_request_refresh([ui] {
        std::cout << "Scanning hardware..." << std::endl;

        bool hardware_found = true; 

        // 修改属性
        ui->set_is_online(hardware_found);
        
        std::cout << "Status updated!" << std::endl;
    });

    ui->run();

    return 0;
}