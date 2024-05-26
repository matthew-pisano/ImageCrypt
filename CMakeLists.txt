cmake_minimum_required(VERSION 3.27)
project(icrypt)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(include lib)

# Source files
set(SOURCES
    src/main.cpp
    src/image_encode.cpp
    src/encodings.cpp
    src/base64.cpp
)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Find ImageMagick
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
include_directories(${ImageMagick_INCLUDE_DIRS})

# Add executable
add_executable(icrypt ${SOURCES})

# Link libraries
target_link_libraries(icrypt ${OpenCV_LIBS} ${ImageMagick_LIBRARIES})
