cmake_minimum_required(VERSION 3.10)
project(SoftRenderer VERSION 1.0.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 对于AppleClang/MacOS的一些优化设置
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    # 启用更多警告
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 包含目录，即头文件位置
include_directories(
    src
    src/core
    src/geometry  
    src/texture
    src/rasterization
)

# 编译这些.cpp文件
add_executable(SoftRenderer
    src/main.cpp
    
    # core
    src/core/FrameBuffer.cpp
    
    # geometry
    src/geometry/Vertex.cpp
    
    # texture
    src/texture/ColorSpace.cpp
    src/texture/YUVTexture.cpp
    
    # rasterization
    src/rasterization/Interpolator.cpp
    src/rasterization/Rasterizer.cpp
)

# 设置输出目录到 build/bin
set_target_properties(SoftRenderer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)