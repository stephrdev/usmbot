cmake_minimum_required(VERSION 3.1)


set(TARGET "usmbot")
set(AVR_MCU "atmega1280")
set(AVR_F_CPU 16000000)

include("$ENV{HOME}/Code/libbbq/AVR.cmake")
project(${TARGET} CXX)

avr_add_firmware(${TARGET} engine.cpp steering.cpp)

avr_add_libraries_directory("$ENV{HOME}/Code/libbbq")
avr_link_library(${TARGET} "bbq_serial")
avr_link_library(${TARGET} "bbq_time")
