cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 23)
project(esmel)


add_executable(esmel main.cpp
        esmel_object.h
        esmel_gc.h
        esmel_interpreter.h
        esmel_callable.h
        esmel_compiler.h)

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
target_link_options(esmel PRIVATE
        -flto
        -Wl,--gc-sections
        -Wl,--strip-all
        -Wl,-O1
        -Wl,--as-needed
        -static
)