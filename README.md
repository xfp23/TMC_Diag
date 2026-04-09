# 🧱 Slint + C++ 项目 CMake 工程结构教程

本教程整理了如何在 Slint + C++ 项目中使用 CMake 进行工程管理，包括：

* 可执行文件命名
* 模块（组件）拆分
* 子目录组织
* 多模块链接

适用于：**嵌入式工程师 / C++工程化开发**

---

# 📌 一、基础工程结构

推荐目录结构：

```
my_application/
├── CMakeLists.txt        # 主工程
├── src/
│   └── main.cpp
├── ui/
│   └── app-window.slint
├── components/
│   └── rtu/
│       ├── CMakeLists.txt
│       ├── include/
│       │   └── rtu.h
│       └── src/
│           └── rtu.cpp
```

---

# 🎯 二、工程名 & 可执行文件

## 基本规则

```cmake
project(my_application)
add_executable(my_application src/main.cpp)
```

👉 生成：

```
my_application.exe
```

---

## 修改 exe 名

```cmake
project(my_tool)
add_executable(my_tool src/main.cpp)
```

---

## 分离工程名与 exe 名（可选）

```cmake
project(core_system)
add_executable(ui_app src/main.cpp)
```

👉 输出：

```
ui_app.exe
```

---

# 🧱 三、主 CMakeLists.txt 模板

```cmake
cmake_minimum_required(VERSION 3.21)

project(my_application LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# 查找 Slint
find_package(Slint REQUIRED)

# 引入组件
add_subdirectory(components/rtu)

# 创建可执行文件
add_executable(my_application src/main.cpp)

# 链接库
target_link_libraries(my_application
    PRIVATE
        Slint::Slint
        rtu
)

# Slint UI
slint_target_sources(my_application ui/app-window.slint)

# Windows 自动复制 DLL
if (WIN32)
    add_custom_command(TARGET my_application POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_RUNTIME_DLLS:my_application>
        $<TARGET_FILE_DIR:my_application>
        COMMAND_EXPAND_LISTS
    )
endif()
```

---

# 🧩 四、组件（模块）写法

## 示例：RTU 模块

路径：

```
components/rtu/CMakeLists.txt
```

内容：

```cmake
add_library(rtu
    src/rtu.cpp
)

target_include_directories(rtu
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

---

## 说明

| 语句             | 作用        |
| -------------- | --------- |
| add_library    | 定义模块      |
| PUBLIC include | 让外部能包含头文件 |

---

# 🔗 五、模块接入主工程

```cmake
add_subdirectory(components/rtu)

target_link_libraries(my_application PRIVATE rtu)
```

---

# 🧪 六、代码中使用

```cpp
#include "rtu.h"

int main()
{
    rtu_init();
}
```

👉 不需要写路径（已自动处理）

---

# 🧱 七、多模块扩展

```
components/
├── rtu/
├── can/
├── modbus/
```

主工程：

```cmake
add_subdirectory(components/rtu)
add_subdirectory(components/can)
add_subdirectory(components/modbus)

target_link_libraries(my_application
    PRIVATE
        rtu
        can
        modbus
)
```

---

# ⚙️ 八、自动收集源码（可选）

```cmake
file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(my_application ${SRC_FILES})
```

---

## 优缺点

| 优点     | 缺点           |
| ------ | ------------ |
| 自动添加文件 | 新文件需重新 cmake |

---

# ⚠️ 九、常见错误

---

## ❌ 错误1：手动写 include 路径

```cpp
#include "../components/rtu/include/rtu.h"
```

✔ 正确：

```cpp
#include "rtu.h"
```

---

## ❌ 错误2：使用 PRIVATE include

```cmake
target_include_directories(rtu PRIVATE ...)
```

👉 会导致主工程找不到头文件

---

# 🧠 十、CMake 核心三句话

```
add_executable     → 生成程序
add_library        → 定义模块
add_subdirectory   → 引入模块
target_link_libraries → 组装系统
```

---

# 🚀 十一、推荐工程架构（进阶）

```
project/
├── app/        # UI层（Slint）
├── core/       # 业务逻辑
├── drivers/    # 驱动层
├── components/ # 功能模块
```

---

# 🎯 总结

* Slint 负责 UI
* CMake 负责组织工程
* 模块通过 `add_library` 拆分
* 主程序通过 `target_link_libraries` 组合

---

# 🧭 下一步建议

可以继续优化：

* UI（Slint）与逻辑解耦
* 引入状态机（适合嵌入式）
* 设计组件复用库

---

👉 本结构已经满足中大型项目开发，可长期使用。
