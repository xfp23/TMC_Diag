
add_library(FileLog STATIC
    src/FileLog.cpp
)

target_include_directories(FileLog PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(FileLog PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

