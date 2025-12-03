cmake_minimum_required(VERSION 3.10)
project(SoftRenderer VERSION 1.0.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_link_options(-std=c++17)
    # 同时为编译器添加标志，确保启用所有 C++17 特性
    add_compile_options(-std=c++17)
endif()

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

# 为编译器预处理器添加宏定义，显式启用 filesystem 特性
target_compile_definitions(SoftRenderer PRIVATE
    __cpp_lib_filesystem=201703L
    PROJECT_ROOT_PATH="${CMAKE_SOURCE_DIR}"  # 关键：定义项目根目录宏
)
# 文件系统库链接
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(SoftRenderer PUBLIC stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_libraries(SoftRenderer PUBLIC c++fs)
endif()

# 设置输出目录到 build/bin
set_target_properties(SoftRenderer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 仅当生成Xcode项目时，设置工作目录
if(CMAKE_GENERATOR STREQUAL "Xcode")
    # 计算项目根目录相对于构建目录的相对路径
    file(RELATIVE_PATH PROJECT_RELATIVE_PATH 
         ${CMAKE_BINARY_DIR}  # 构建目录 (e.g., build/xcode-Debug)
         ${CMAKE_SOURCE_DIR}  # 项目根目录
    )
    
    # Debug配置：使用项目根目录，以便运行时能找到资源文件
    set(CMAKE_XCODE_ATTRIBUTE_WORKING_DIRECTORY[config=Debug] 
        "\$(SRCROOT)/${PROJECT_RELATIVE_PATH}"
    )
    
    # Release配置：使用可执行文件目录（生产部署）
    set(CMAKE_XCODE_ATTRIBUTE_WORKING_DIRECTORY[config=Release] 
        "\$(BUILT_PRODUCTS_DIR)")
    
    message(STATUS "已配置Xcode Debug工作目录: \$(SRCROOT)/${PROJECT_RELATIVE_PATH}")
endif()