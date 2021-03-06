cmake_minimum_required(VERSION 3.17)
project(elevator_sample)

set(CMAKE_CXX_STANDARD 14)

add_library(elevator_sample SHARED lib_evr.cpp lib_evr.h)