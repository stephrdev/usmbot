cmake_minimum_required(VERSION 3.1)


set(TARGET "usmbot")
set(AVR_MCU "atmega1280")
set(AVR_F_CPU 16000000)

include("/Users/steph/Desktop/bbq/AVR.cmake")
project(${TARGET} CXX)

avr_add_firmware(${TARGET})

avr_add_libraries_directory("/Users/steph/Desktop/bbq")
avr_link_library(${TARGET} "bbq_serial")
