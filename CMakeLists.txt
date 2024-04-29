cmake_minimum_required(VERSION 3.27)
project(icrypt)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/include/opencv4)
find_package( OpenCV REQUIRED )
include_directories(include lib)

add_executable(icrypt src/main.cpp
        src/image_encode.cpp
        include/image_encode.h
        src/encodings.cpp
        include/encodings.h
        lib/CLI11/CLI11.hpp)

target_link_libraries(icrypt ${OpenCV_LIBS})
