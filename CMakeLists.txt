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
        lib/CLI11/CLI11.hpp
        src/base64.cpp
        include/base64.h)

target_link_libraries(icrypt ${OpenCV_LIBS})

find_package(Catch2 3 REQUIRED)
add_executable(tests
        src/base64.cpp
        test/test_base64.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)
