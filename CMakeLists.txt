cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 23)
project(esmel)

add_executable(esmel main.cpp
        esmel_object.h
        esmel_gc.h
        esmel_interpreter.h
        esmel_callable.h
        esmel_compiler.h
        esmel_exception.h)