# SoftRenderer
Soft renderer, for personal learning and practice of rendering knowledge.

[![CMake](https://github.com/Jormungand-Stark/SoftRenderer/actions/workflows/cmake.yml/badge.svg)](https://github.com/Jormungand-Stark/SoftRenderer/actions/workflows/cmake.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## 特性
- 完整的软渲染管线实现
- YUV420纹理支持
- 重心坐标光栅化
- 可扩展的架构设计

## 快速开始
### 1. 克隆与构建
```bash
git clone https://github.com/Jormungand-Stark/SoftRenderer
cd SoftRenderer
mkdir build && cd build
cmake .. && make
```
### 2. 运行程序
```bash
# 从build目录运行（开发者）
./bin/SoftRenderer 
# 或指定YUV文件：
./bin/SoftRenderer ../assets/yuv/test_320x240.yuv 320 240

# 从项目根目录运行（用户）
./build/bin/SoftRenderer
# 或指定YUV文件：
./build/bin/SoftRenderer assets/yuv/test_320x240.yuv 320 240
```

### 3. 输出位置
- 程序运行后，渲染生成的RGB图像（PPM格式）将自动保存到项目根目录的 samples/ 文件夹中。
- 无论从哪里运行，输出都在同一位置。

## 测试资源
### 预置测试文件
- assets/yuv/test_320x240.yuv - 320×240 渐变图案（~115 KB）
- assets/yuv/test_640x480.yuv - 640×480 渐变图案（~460 KB）

### 生成自定义测试文件
项目提供了Python脚本用于生成I420格式的YUV测试文件。

#### 1. 安装依赖
首先确保已安装所需的Python库：

```bash
pip install numpy
```

#### 2. 使用说明
在终端中，首先切换到 SoftRenderer 项目根目录，然后运行脚本：

```bash
# 生成320x240测试文件
python assets/scripts/generate_test_yuv.py 320 240 assets/yuv/test_320x240.yuv

# 生成640x480测试文件  
python assets/scripts/generate_test_yuv.py 640 480 assets/yuv/test_640x480.yuv
```

#### 3. 从任何位置运行
如果不在项目根目录，可以使用脚本和输出文件的绝对路径：

```bash
python /path/to/SoftRenderer/assets/scripts/generate_test_yuv.py 640 480 /path/to/SoftRenderer/assets/yuv/output.yuv
```

#### ⚠️ 注意事项
- **脚本会自动创建不存在的输出目录**。
- 建议使用偶数宽高，脚本会对奇数值进行+1调整（YUV420格式要求）。
- 运行后会打印生成文件的详细信息和使用示例。

## 依赖管理

本项目将第三方库作为源码直接嵌入（Vendor），以保证构建的可重现性。

### 核心依赖
- **[GLM](https://github.com/g-truc/glm)** (OpenGL Mathematics)：用于图形数学运算的头文件库。
  - **版本**：1.0.2
  - **位置**：`third_party/glm/`
  - **管理方式**：直接嵌入头文件（非 Git 子模块）。

### 注意事项
- 所有依赖均位于 `third_party/` 目录，构建系统（CMake）已配置好包含路径。
- 若需升级 GLM，请下载其官方发布包（Release），替换 `third_party/glm/` 目录内容，并更新此文档中的版本号。

## 项目结构
```text
SoftRenderer/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── Color.hpp      # 纯头文件
│   │   ├── FrameBuffer.hpp
│   │   └── FrameBuffer.cpp
│   ├── geometry/
│   │   ├── Vertex.hpp
│   │   └── Vertex.cpp
│   ├── texture/
│   │   ├── ColorSpace.hpp
│   │   ├── ColorSpace.cpp
│   │   ├── YUVTexture.hpp
│   │   └── YUVTexture.cpp
│   └── rasterization/
│       ├── Interpolator.hpp
│       ├── Interpolator.cpp
│       ├── Rasterizer.hpp
│       └── Rasterizer.cpp
└── build/                  # 用户创建的构建目录
    └── bin/
        └── SoftRenderer    # 生成的可执行文件
```

## 技术栈
- C++17
- CMake
- STL

## 学习要点
通过本项目可以学习：
1. 软件渲染管线架构
2. 三角形光栅化算法
3. 重心坐标插值
4. YUV色彩空间转换
5. 纹理映射原理

## 许可证
MIT License - 查看 [LICENSE](LICENSE) 文件了解详情

## CI/CD 构建说明

### 已知问题与解决

**问题**：GitHub Actions CI 构建失败，报错 `fatal error: glm/glm.hpp: No such file or directory`。

**原因**：`third_party/glm` 目录曾被错误地识别为 Git 子模块，导致 CI 服务器无法获取其源码。

**解决方案**：
1.  确保 `glm` 作为纯头文件库嵌入在 `third_party/` 目录中，**不是**子模块。
2.  确保 CMake 包含路径于项目 `CMakeLists.txt` 中已正确设置 ：
    ```cmake
    target_include_directories(SoftRenderer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm)
    ```