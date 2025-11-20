//
//  main.cpp
//  softrender
//
//  Created by 封睿文 on 2025/11/19.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

// 定义颜色结构体，存储 RGB 三通道（每个通道 0-255）
struct Color {
    unsigned char r, g, b;
};

// 顶点结构体，包含二维坐标和颜色
struct Vertex {
    float x, y;   // 顶点坐标（屏幕空间）
    float u, v;   // 纹理坐标
    // NOTE: 在纹理渲染中可能不需要顶点颜色 color, 因为颜色来自纹理采样
    // Color color;  // 顶点颜色
};

struct YUVTexture {
    /// 为什么用std::vector<unsigned char>而不是float？
    /// 因为unsigned char兼顾了传输性能和图像质量，一般来讲8bit对于视觉上能接受的图片精度已然足够，
    /// 除非进行复杂的数字信号处理（颜色空间变换、hdr、滤镜渲染等）才需要将8bit数据转成flaot类型32bit。
    // I420格式，Y、U、V三个独立平面
    std::vector<unsigned char> y_plane;
    std::vector<unsigned char> u_plane;
    std::vector<unsigned char> v_plane;
    int width, height; // 宽高
    
    // 从文件加载数据
    YUVTexture(const std::string& file_path, int w, int h);
    
    // 根据纹理坐标获取YUV值
    void sampleYUV(float u, float v, unsigned char& y, unsigned char& u_val, unsigned char& v_val);
};

// 帧缓冲，用来存储屏幕上的像素数据
struct FrameBuffer {
    int width, height;              // 分辨率
    std::vector<Color> pixels;      // 存储渲染结果，像素数组，大小 = width * height
    
    // 需要哪些方法？
    // 1. 清屏
    // 2. 设置像素
    // 3. 保存为图片
    // 4. 其他？

    FrameBuffer(int w, int h) : width(w), height(h), pixels(w * h) {}

    // 清屏函数，把整个帧缓冲填充为指定颜色
    

    // 设置某个像素点的颜色

    // 保存帧缓冲为 PPM 图片文件（简单的 RGB 格式）
};


int main() {
    std::cout << "Soft Renderer Started!" << std::endl;
    return 0;
}
