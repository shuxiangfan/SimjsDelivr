cmake_minimum_required(VERSION 4.0)
project(SimjsDeliver)

set(CMAKE_CXX_STANDARD 26)

add_executable(SimjsDeliver main.cpp
        file_download.cpp
        decompressor.cpp
)
target_link_libraries(SimjsDeliver curl)
target_link_libraries(SimjsDeliver archive)
