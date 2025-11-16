cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 23)
project(esmel)

#set(CMAKE_EXE_LINKER_FLAGS "-static")

#if(MSVC)
#    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#add_compile_options("/O2" "/std:c++20")
#else()
#endif()
#
#if(WIN32)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0A00")
#    target_link_libraries(kernel32 user32 gdi32 advapi32 shell32)
#endif()

add_executable(esmel main.cpp
        gc.h)