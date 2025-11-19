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
    Color color;  // 顶点颜色
};

// 帧缓冲，用来存储屏幕上的像素数据
struct FrameBuffer {
    int width, height;              // 分辨率
    std::vector<Color> pixels;      // 像素数组，大小 = width * height

    FrameBuffer(int w, int h) : width(w), height(h), pixels(w * h) {}

    // 清屏函数，把整个帧缓冲填充为指定颜色
    void clear(Color c) {
        std::fill(pixels.begin(), pixels.end(), c);
    }

    // 设置某个像素点的颜色
    void setPixel(int x, int y, Color c) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[y * width + x] = c;
        }
    }

    // 保存帧缓冲为 PPM 图片文件（简单的 RGB 格式）
    void savePPM(const std::string& filename) {
        std::ofstream ofs(filename, std::ios::binary);
        ofs << "P6\n" << width << " " << height << "\n255\n"; // PPM 文件头
        for (auto& p : pixels) {
            ofs << p.r << p.g << p.b; // 写入每个像素的 RGB 值
        }
        ofs.close();
    }
};

// 计算点 (px, py) 在三角形 (v0, v1, v2) 中的重心坐标
// 返回 w0, w1, w2 三个权重，如果点在三角形内则返回 true
bool barycentric(float px, float py, const Vertex& v0, const Vertex& v1, const Vertex& v2,
                 float& w0, float& w1, float& w2) {
    // 计算分母（面积相关）
    float denom = (v1.y - v2.y)*(v0.x - v2.x) + (v2.x - v1.x)*(v0.y - v2.y);
    // 计算重心坐标
    w0 = ((v1.y - v2.y)*(px - v2.x) + (v2.x - v1.x)*(py - v2.y)) / denom;
    w1 = ((v2.y - v0.y)*(px - v2.x) + (v0.x - v2.x)*(py - v2.y)) / denom;
    w2 = 1.0f - w0 - w1;
    // 判断是否在三角形内部（所有权重 >= 0）
    return (w0 >= 0 && w1 >= 0 && w2 >= 0);
}

// 绘制三角形，使用重心坐标插值颜色
void drawTriangle(FrameBuffer& fb, const Vertex& v0, const Vertex& v1, const Vertex& v2) {
    // 计算三角形的包围盒（bounding box）
    int minX = std::min({(int)v0.x, (int)v1.x, (int)v2.x});
    int maxX = std::max({(int)v0.x, (int)v1.x, (int)v2.x});
    int minY = std::min({(int)v0.y, (int)v1.y, (int)v2.y});
    int maxY = std::max({(int)v0.y, (int)v1.y, (int)v2.y});

    // 遍历包围盒内的所有像素
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            float w0, w1, w2;
            // 判断像素是否在三角形内
            if (barycentric(x+0.5f, y+0.5f, v0, v1, v2, w0, w1, w2)) {
                // 插值计算颜色
                Color c;
                c.r = (unsigned char)(w0*v0.color.r + w1*v1.color.r + w2*v2.color.r);
                c.g = (unsigned char)(w0*v0.color.g + w1*v1.color.g + w2*v2.color.g);
                c.b = (unsigned char)(w0*v0.color.b + w1*v1.color.b + w2*v2.color.b);
                // 设置像素颜色
                fb.setPixel(x, y, c);
            }
        }
    }
}

int main() {
    // 创建一个 400x400 的帧缓冲
    FrameBuffer fb(400, 400);
    fb.clear({255, 255, 255}); // 清屏为白色背景
    
    // 定义三角形三个顶点（带颜色）
    Vertex v0 = {50, 50, {255, 0, 0}};   // 红色顶点
    Vertex v1 = {350, 100, {0, 255, 0}}; // 绿色顶点
    Vertex v2 = {200, 300, {0, 0, 255}}; // 蓝色顶点
    
    // 绘制三角形
    drawTriangle(fb, v0, v1, v2);
    
    // 保存结果到 output.ppm
    fb.savePPM("/Users/jormungand/Downloads/render.ppm");
    std::cout << "Saved to output.ppm\n";
    return 0;
}
