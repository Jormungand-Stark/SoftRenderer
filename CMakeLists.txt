cmake_minimum_required(VERSION 3.10)
project(SoftRenderer)

set(CMAKE_CXX_STANDARD 11)

# 源代码文件
set(SRC_FILES
    src/main.cpp
    src/core/Color.cpp
    src/core/FrameBuffer.cpp
    src/texture/YUVTexture.cpp
    src/texture/ColorSpace.cpp
    src/rasterization/Rasterizer.cpp
)

# 包含目录
include_directories(
    src/core
    src/geometry
    src/texture
    src/rasterization
)

add_executable(SoftRenderer ${SRC_FILES})