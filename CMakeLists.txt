cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 23)
project(esmel)

add_executable(esmel main.cpp
        esmel_object.h
        esmel_gc.h
        esmel_interpreter.h
        esmel_callable.h
        esmel_compiler.h
        fast_float.h)

target_compile_options(esmel PRIVATE
        -O2
        -flto
        -fno-exceptions
        -fno-rtti
        -ffunction-sections
        -fdata-sections
        -march=native
        -mtune=native
        -fomit-frame-pointer
        -Wall
        -Wextra
)

# 根据操作系统设置不同的链接选项
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(esmel PRIVATE
            -flto
            -Wl,--gc-sections
            -Wl,--strip-all
            -Wl,-O1
            -Wl,--as-needed
            -static
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")  # macOS
    target_link_options(esmel PRIVATE
            -flto
            # macOS 对应的选项
            -Wl,-dead_strip           # 相当于 --gc-sections
            -Wl,-S                    # 相当于 --strip-all（剥离调试符号）
            -Wl,-O1                   # 链接器优化（macOS 也支持）
            # -Wl,--as-needed         # macOS 没有直接对应项
            # -static                 # macOS 不推荐静态链接，除非特殊需要
    )
    # 如果需要完全 strip（去除所有符号），添加 post build 命令
    add_custom_command(TARGET esmel POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:esmel>
        COMMENT "Stripping symbols"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_options(esmel PRIVATE
            -flto
            -static
    )
endif()