cmake_minimum_required(VERSION 3.15)
project(pointer_tagging)

set(CMAKE_CXX_STANDARD 17)

add_executable(pointer_tagging main.cpp PTRTaggingUtil.cpp)