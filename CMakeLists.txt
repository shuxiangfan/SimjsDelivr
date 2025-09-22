cmake_minimum_required(VERSION 4.0)
project(SimjsDeliver)

set(CMAKE_CXX_STANDARD 26)

add_executable(SimjsDeliver main.cpp
        Updater.cpp
        Updater.h)
find_package(libcurl-simple-https CONFIG REQUIRED)
target_link_libraries(main PRIVATE libcurl-simple-https)