cmake_minimum_required(VERSION 3.17)

project(OS_vfs_debug)

include_directories(include)
include_directories(log)
include_directories(vfs)

file(GLOB SOURCES "res/debug_vfs.cpp")

add_executable(debug_vfs ${SOURCES})
