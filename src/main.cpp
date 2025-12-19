//
//  main.cpp
//  softrender
//
//  Created by Jormungand on 2025/11/19.
//

#ifdef _WIN32
    #include <windows.h>
    // 在Windows上，等价的目录切换函数是 _chdir，位于 <direct.h>
    #define chdir _chdir
    #include <direct.h>
#else
    // 对于macOS、Linux及其他类Unix系统，chdir函数都在 <unistd.h> 中
    #include <unistd.h>
    #ifdef __APPLE__
        #include <mach-o/dyld.h> // 仅macOS需要这个来获取可执行文件路径
    #endif
#endif

#include <iostream>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <core/FrameBuffer.hpp>
#include <texture/YUVTexture.hpp>
#include <shaders/Transform2DShader.hpp>
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

#if 1

// 创建4x4棋盘格YUV文件（I420格式）
void createTestChessYUV(const std::string& filename, int width = 4, int height = 4) {
    std::ofstream file(filename, std::ios::binary);
    
    // Y平面（亮度） - 棋盘格
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char y_val = ((x + y) % 2 == 0) ? 0 : 255; // 棋盘格
            file.write(reinterpret_cast<char*>(&y_val), 1);
        }
    }
    
    // U平面（中性灰）
    for (int y = 0; y < height/2; y++) {
        for (int x = 0; x < width/2; x++) {
            unsigned char u_val = 128;
            file.write(reinterpret_cast<char*>(&u_val), 1);
        }
    }
    
    // V平面（中性灰）
    for (int y = 0; y < height/2; y++) {
        for (int x = 0; x < width/2; x++) {
            unsigned char v_val = 128;
            file.write(reinterpret_cast<char*>(&v_val), 1);
        }
    }
    
    file.close();
    std::cout << "创建测试纹理: " << filename 
              << " (" << width << "x" << height << ")" << std::endl;
}

std::vector<Vertex> createFullscreenQuad(float screenWidth, float screenHeight) {
    return {
        // 三角形1：左下、右下、左上
        {0.0f, 0.0f, 0.0f, 0.0f},           // 左下，纹理坐标(0,0)
        {screenWidth, 0.0f, 1.0f, 0.0f},    // 右下，纹理坐标(1,0)  
        {0.0f, screenHeight, 0.0f, 1.0f},   // 左上，纹理坐标(0,1)
        
        // 三角形2：右下、右上、左上
        {screenWidth, 0.0f, 1.0f, 0.0f},    // 右下
        {screenWidth, screenHeight, 1.0f, 1.0f}, // 右上
        {0.0f, screenHeight, 0.0f, 1.0f}    // 左上
    };
}

int runFilterTest() {
    std::cout << "=== 运行过滤对比测试 ===" << std::endl;
    
    // 获取当前目录
    std::string current_dir = std::filesystem::current_path().string();
    std::string test_dir = "samples/test";
    
    // 如果在build目录，调整路径
    if (current_dir.find("build") != std::string::npos) {
        test_dir = "../samples/test";
    }
    
    // 创建测试目录
    std::filesystem::create_directories(test_dir);
    
    // 创建测试纹理（4x4棋盘格）
    std::string texture_path = test_dir + "/test_chess_4x4.yuv";
    createTestChessYUV(texture_path, 4, 4);
    
    // 纹理尺寸和屏幕尺寸
    int textureWidth = 4;
    int textureHeight = 4;
    int screenWidth = 800;
    int screenHeight = 600;
    
    // 创建纹理对象
    SoftRenderer::YUVTexture texture(texture_path, textureWidth, textureHeight);
    
    // 创建光栅化器
    SoftRenderer::Rasterizer rasterizer;
    
    // ========== 测试1：最邻近过滤 ==========
    {
        std::cout << "\n测试1: 最邻近过滤 (Nearest)" << std::endl;
        
        SoftRenderer::FrameBuffer fb(screenWidth, screenHeight);
        fb.clear({128, 128, 128}); // 灰色背景
        
        texture.setFilterMode(SoftRenderer::TextureFilter::NEAREST);  // 注意：你的枚举可能是NEAREST
        
        auto quad = createFullscreenQuad(fb.getWidth(), fb.getHeight());
        
        // 绘制两个三角形
        rasterizer.drawTexturedTriangle(fb, quad[0], quad[1], quad[2], texture);
        rasterizer.drawTexturedTriangle(fb, quad[3], quad[4], quad[5], texture);
        
        std::string output_path = test_dir + "/nearest_4x4_to_800x600.ppm";
        fb.saveToPPM(output_path);
        std::cout << "已保存: " << output_path << std::endl;
    }
    
    // ========== 测试2：双线性过滤 ==========
    {
        std::cout << "\n测试2: 双线性过滤 (Bilinear)" << std::endl;
        
        SoftRenderer::FrameBuffer fb(screenWidth, screenHeight);
        fb.clear({128, 128, 128}); // 灰色背景
        
        texture.setFilterMode(SoftRenderer::TextureFilter::BILINEAR);
        
        auto quad = createFullscreenQuad(fb.getWidth(), fb.getHeight());
        
        // 绘制两个三角形
        rasterizer.drawTexturedTriangle(fb, quad[0], quad[1], quad[2], texture);
        rasterizer.drawTexturedTriangle(fb, quad[3], quad[4], quad[5], texture);
        
        std::string output_path = test_dir + "/bilinear_4x4_to_800x600.ppm";
        fb.saveToPPM(output_path);
        std::cout << "已保存: " << output_path << std::endl;
    }
    
    // ========== 调试信息 ==========
    std::cout << "\n=== 测试信息 ===" << std::endl;
    std::cout << "纹理尺寸: " << textureWidth << "x" << textureHeight << std::endl;
    std::cout << "屏幕尺寸: " << screenWidth << "x" << screenHeight << std::endl;
    std::cout << "放大倍数: " << (screenWidth/textureWidth) << "倍" << std::endl;
    std::cout << "\n✅ 测试完成！" << std::endl;
    std::cout << "用图片查看器打开以下文件并放大观察：" << std::endl;
    std::cout << "1. " << test_dir << "/nearest_4x4_to_800x600.ppm" << std::endl;
    std::cout << "2. " << test_dir << "/bilinear_4x4_to_800x600.ppm" << std::endl;
    std::cout << "\n提示：按 Ctrl+滚轮 或 Cmd+加号 放大图片" << std::endl;
    
    return 0;
}

int main() {
    try {
        runFilterTest();
        std::cout << "\n✅ 测试完成！" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ 测试失败: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}

#else  // 正常的渲染程序

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
        
        // 2. 加载YUV纹理
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
        texture.setFilterMode(SoftRenderer::TextureFilter::BILINEAR); // 使用双线性过滤
        
        // 3. 创建和配置顶点着色器
        auto vertex_shader = std::make_unique<SoftRenderer::Transform2DShader>();
        SoftRenderer::Transform2DUniforms uniforms;
        uniforms.translateX = 100.0f; // 向右平移100像素
        // uniforms.translateY = 0.0f;
        uniforms.scaleX = 2.0f; // 宽度放大2倍（非均匀缩放）
        uniforms.scaleY = 1.0f; // 高度保持不变
        uniforms.rotate_angle = glm::radians(45.0f); // 旋转45度

        vertex_shader->setUniforms(uniforms);

        // 4. 定义原始顶点数据
        std::vector<Vertex> original_vertices = {
            // 屏幕坐标   // 纹理坐标
            // 左下三角形
            {0.0f, 0.0f, 0.0f, 0.0f},     // 左下
            {800.0f, 0.0f, 1.0f, 0.0f},   // 右下
            {0.0f, 600.0f, 0.0f, 1.0f},   // 左上
            // 右上三角形
            {0.0f, 600.0f, 0.0f, 1.0f},   // 左上
            {800.0f, 600.0f, 1.0f, 1.0f}, // 右上
            {800.0f, 0.0f, 1.0f, 0.0f}    // 右下
        };

        // 5. 应用顶点着色器变换
        std::vector<Vertex> transformed_vertices(original_vertices.size());
        vertex_shader->processVertices(
            transformed_vertices.data(), 
            original_vertices.data(), 
            original_vertices.size()
        );

        // 6. 创建光栅化器并渲染变换后的三角形
        SoftRenderer::Rasterizer rasterizer;
        
        // 使用变换后的顶点进行渲染！
        rasterizer.drawTexturedTriangle(fb, transformed_vertices[0], transformed_vertices[1], transformed_vertices[2], texture);
        rasterizer.drawTexturedTriangle(fb, transformed_vertices[3], transformed_vertices[4], transformed_vertices[5], texture);
        
        // 绘制纯色三角形，debug code
//        rasterizer.drawSolidTriangle(fb, quad[0], quad[1], quad[2], {255, 0, 0});
//        rasterizer.drawSolidTriangle(fb, quad[3], quad[4], quad[5], {0, 255, 0});
        
        // 7. 保存结果（智能输出路径）
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

#endif
