cmake_minimum_required(VERSION 4.1)
project(imageChanger)

set(CMAKE_CXX_STANDARD 20)

add_executable(imageChanger main.cpp
        main.hpp
        ImageConverter.cpp
        ImageConverter.hpp)

file(COPY ${CMAKE_SOURCE_DIR}/Images DESTINATION ${CMAKE_BINARY_DIR})
