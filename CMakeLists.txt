cmake_minimum_required(VERSION 3.10)

project(CircularBuffer)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_definitions(-D_XOPEN_SOURCE=500)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")

include_directories(inc)

find_library(LIB_RT rt)      
find_library(LIB_M m)       
find_package(Threads REQUIRED) 

add_executable(vSensor src/vSensor.c)
target_link_libraries(vSensor ${LIB_RT} ${LIB_M} Threads::Threads)

add_executable(monitor_avg src/monitor_avg.c)
target_link_libraries(monitor_avg ${LIB_RT} Threads::Threads)

add_executable(monitor_f src/monitor_f.c)
target_link_libraries(monitor_f ${LIB_RT} ${LIB_M} Threads::Threads)