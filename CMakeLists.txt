cmake_minimum_required(VERSION 3.21)

# ===== 工程名统一 =====
set(PROJECT_NAME TMC_Diag)

project(${PROJECT_NAME} LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ===== UTF-8 =====
if(MSVC)
    add_compile_options(/utf-8)
endif()

# set(CMAKE_GENERATOR_PLATFORM Win32)

# ===== Slint =====
find_package(Slint REQUIRED)

# ===== 子模块 =====
add_subdirectory(components)

# ===== 收集 Global 源码 =====
file(GLOB_RECURSE GLOBAL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Global/src/*.cpp
)

file(GLOB_RECURSE SlintWindow_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/SlintWindow/src/*.cpp
)

# ===== 可执行文件 =====
add_executable(${PROJECT_NAME}
    src/main.cpp
    ui/resources.rc
    ${GLOBAL_SRC}
    ${SlintWindow_SRC}
)

# ===== Slint UI =====
target_link_libraries(${PROJECT_NAME} PRIVATE
    Slint::Slint
    USBCAN_2E_U
    FileLog
)

slint_target_sources(${PROJECT_NAME} ui/app-window.slint)

# ===== include =====
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Global/include
    ${CMAKE_CURRENT_SOURCE_DIR}/SlintWindow/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ===== 关键：GUI + main() 共存 =====
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE YES
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
endif()

# ===== 自动复制 DLL（关键）=====
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
endif()