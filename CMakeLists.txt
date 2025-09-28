cmake_minimum_required(VERSION 4.0)
project(SimjsDelivr)

set(CMAKE_CXX_STANDARD 26)

add_executable(SimjsDelivr main.cpp
        file_net_stream.cpp
        decompressor.cpp
        get_content_type.cpp
        filelist.cpp
        main.h
        response_parser.cpp
        cache_system.cpp
)
target_link_libraries(SimjsDelivr curl)
target_link_libraries(SimjsDelivr archive)
target_link_libraries(SimjsDelivr fmt)
target_compile_options(SimjsDelivr PUBLIC "-O2" "-flto=auto")