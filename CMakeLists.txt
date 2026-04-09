cmake_minimum_required(VERSION 3.20)

# 项目名称
project(ZLG_USBCAN_2E_U LANGUAGES CXX)

# 1. 查找 Slint 包
find_package(Slint REQUIRED)

# 2. 定义目标名称变量
set(PROJECT_TARGET ZLG_USBCAN_2E_U)

# 3. 创建可执行文件 (加上 WIN32 隐藏黑窗口)
add_executable(${PROJECT_TARGET} WIN32 src/main.cpp)

# 4. 修复 LNK2019 WinMain 报错：强制指定入口点为 main
if(MSVC)
    set_target_properties(${PROJECT_TARGET} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

# 5. 链接 Slint 库
target_link_libraries(${PROJECT_TARGET} PRIVATE Slint::Slint)

# 6. 编译 Slint UI 文件
slint_target_sources(${PROJECT_TARGET} ui/app-window.slint)

# 7. Windows 自动复制运行所需的 DLL (如 slint.dll) 到 exe 所在目录
if (WIN32)
    add_custom_command(TARGET ${PROJECT_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_RUNTIME_DLLS:${PROJECT_TARGET}>
        $<TARGET_FILE_DIR:${PROJECT_TARGET}>
        COMMAND_EXPAND_LISTS
    )
endif()