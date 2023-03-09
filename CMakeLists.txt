cmake_minimum_required(VERSION 3.25)
project(win32)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(win32  main.cpp)

target_link_libraries(win32 PRIVATE fmt::fmt Boost::program_options)