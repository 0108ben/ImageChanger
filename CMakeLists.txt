cmake_minimum_required(VERSION 4.1)
project(imageChanger)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)

add_executable(imageChanger main.cpp
        main.hpp
        ImageConverter.cpp
        ImageConverter.hpp
        renderer.cpp
        renderer.hpp)

file(COPY ${CMAKE_SOURCE_DIR}/Images DESTINATION ${CMAKE_BINARY_DIR})

target_link_libraries(imageChanger PRIVATE SFML::Network SFML::Graphics SFML::Window SFML::Audio SFML::System)
