//
//  main.cpp
//  softrender
//
//  Created by Jormungand on 2025/11/19.
//

#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#elif defined(__linux__)
    #include <unistd.h>
#endif

#include <iostream>
#include <filesystem>

#include <core/FrameBuffer.hpp>
#include <texture/YUVTexture.hpp>
#include <rasterization/Rasterizer.hpp>

// 获取项目根目录
std::string getProjectRoot() {
    namespace fs = std::filesystem;
    
    // 1. 编译时定义（最可靠）
    #ifdef PROJECT_ROOT_PATH
    return PROJECT_ROOT_PATH;
    #endif
    
    // 2. 环境变量
    if (const char* env_root = std::getenv("SOFTRENDERER_ROOT")) {
        return env_root;
    }
    
    // 3. 可执行文件位置推导（macOS特化）
    #ifdef __APPLE__
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        fs::path exe_path = fs::canonical(path);
        // 向上查找CMakeLists.txt
        for (fs::path current = exe_path;
             current != current.root_path();
             current = current.parent_path()) {
            if (fs::exists(current / "CMakeLists.txt")) {
                return current.string();
            }
        }
    }
    #endif
    
    // 4. 默认：当前目录
    return fs::current_path().string();
}


int main(int argc, char* argv[]) {
    /**
     YUVTexture (YUV数据)
          ↓
     纹理采样 → 得到YUV值
          ↓
     色彩空间转换 → YUV→RGB
          ↓
     FrameBuffer (写入RGB结果)
          ↓
     保存为图片/显示
     */
    try {
        // 获取和打印更多环境信息
        std::cout << "=== Xcode环境调试 ===" << std::endl;
        
        std::string project_root = getProjectRoot();
        
        std::cout << "项目根目录: " << project_root << std::endl;
        
        if (chdir(project_root.c_str()) != 0) {
            std::cerr << "警告: 无法切换到项目根目录" << std::endl;
        } else {
            std::cout << "已切换到: " << std::filesystem::current_path() << std::endl;
        }
        
        // 现在可以正确访问assets
        std::string yuv_path = "assets/yuv/test_640x480.yuv";
        std::cout << "YUV文件: " << yuv_path << std::endl;
        std::cout << "文件存在: " << std::filesystem::exists(yuv_path) << std::endl;
        
        // 1. 创建帧缓冲作为输出目标
        SoftRenderer::FrameBuffer fb(800, 600);
        fb.clear();
        
        // 2. 加载YUV纹理（智能默认路径）
        std::string input_file;
        
        // 获取当前工作目录
        std::string current_dir = std::filesystem::current_path().string();
        if (argc > 1) {
            input_file = argv[1]; // 从命令行获取
        }
        else {
            // 智能选择默认路径
            if (current_dir.find("build") != std::string::npos) {
                // 在build目录运行 → 向上找assets
                input_file = "../assets/yuv/test_640x480.yuv";
            } else {
                // 在项目根目录或其他目录运行 → 直接找assets
                input_file = "assets/yuv/test_640x480.yuv";
            }
        }

        if (!std::filesystem::exists(input_file)) {
            std::cerr << "错误: 找不到输入文件 " << input_file << std::endl;
            std::cerr << "请运行: python assets/scripts/generate_test_yuv.py 640 480 " << input_file << std::endl;
            return 1;
        }

        SoftRenderer::YUVTexture texture(input_file, 640, 480);   
        
        // 3. 光栅化
        SoftRenderer::Rasterizer rasterizer;
        
        // 4. 创建全屏四边形顶点
        std::vector<Vertex> quad = {
            // 屏幕坐标   // 纹理坐标
            // 左下三角形
            {0.0f, 0.0f, 0.0f, 0.0f},
            {800.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 600.0f, 0.0f, 1.0f},
            // 右上三角形
            {0.0f, 600.0f, 0.0f, 1.0f},
            {800.0f, 600.0f, 1.0f, 1.0f},
            {800.0f, 0.0f, 1.0f, 0.0f}
        };
        
        // 5. 渲染两个三角形
        rasterizer.drawTexturedTriangle(fb, quad[0], quad[1], quad[2], texture);
        rasterizer.drawTexturedTriangle(fb, quad[3], quad[4], quad[5], texture);
        
        // 绘制纯色三角形，debug code
//        rasterizer.drawSolidTriangle(fb, quad[0], quad[1], quad[2], {255, 0, 0});
//        rasterizer.drawSolidTriangle(fb, quad[3], quad[4], quad[5], {0, 255, 0});
        
        // 6. 保存结果（智能输出路径）
        std::string output_dir = "samples";
        // 如果当前目录包含"build"，说明在build目录运行，需要向上找samples目录
        if (current_dir.find("build") != std::string::npos) {
            output_dir = "../samples";
        }

        // 确保输出目录存在
        std::filesystem::create_directories(output_dir);

        std::string output_file = output_dir + "/render_" +
                                 std::to_string(fb.getWidth()) + "x" +
                                 std::to_string(fb.getHeight()) + ".ppm";
        
        if (fb.saveToPPM(output_file)) {
            std::cout << "✅ 渲染成功!" << std::endl;
            std::cout << "   输入: " << input_file << std::endl;
            std::cout << "   输出: " << output_file << std::endl;
            std::cout << "   尺寸: " << fb.getWidth() << "x" << fb.getHeight() << std::endl;
        } else {
            std::cerr << "❌ 保存失败: " << output_file << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
