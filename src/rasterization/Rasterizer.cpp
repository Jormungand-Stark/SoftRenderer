//
//  Rasterizer.cpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#include "Rasterizer.hpp"
#include <cmath>
#include <algorithm>

namespace SoftRenderer {

Rasterizer::Rasterizer(int w, int h) : width(w), height(h) {
    // 由于 Vertex 和 Rasterizer 都不能更改宽高成员变量的类型，因此构造时需要将 int 转换为 float。
    float w_f = static_cast<float>(width);
    float h_f = static_cast<float>(height);
    
    // 创建两个三角形表示矩形面片
    std::vector<Vertex> vertices = {
        // 三角形1：左下、右下、左上
        {0.0f, 0.0f, 0.0f, 0.0f}, // 左下
        {w_f, 0.0f, 1.0f, 0.0f},  // 右下
        {0.0f, h_f, 0.0f, 1.0f},  // 左上
        
        // 三角形2：右下、右上、左上
        {w_f, 0.0f, 1.0f, 0.0f},  // 右下
        {w_f, h_f, 1.0f, 1.0f},   // 右上
        {0.0f, h_f, 0.0f, 1.0f}   // 左上
    };
}

// 二维向量叉积公式: (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0)
// 辅助函数：
// 1. 公式返回的标量值代表以 P0P1 和 P0P2 两个向量（三角形邻边）所围成的平行四边形的有向面积
// 2. 因此，该值是对应三角形 (P0, P1, P2)/ (v0, v1, v2) 面积的两倍。
// 3. 该有向面积的正负号可用于判断点的内外关系（例如光栅化优化）：
//      - 如果三个叉乘的结果符号相同，则点 P 在三角形内部。
//      - 如果存在任何一个叉乘结果为负，则点 P 在三角形外部。
//      - 如果任何一个叉乘结果为零，则点 P 在三角形的某条边上。
inline float edgeFunction(float x0, float y0, float x1, float y1, float x2, float y2) {
    return (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
}

float Rasterizer::computeBarycentric(float Px, float Py, const Vertex& v0,
                                     const Vertex& v1, const Vertex& v2,
                                     float& w0, float& w1, float& w2) {
    
    // TotalArea_2X = edgeFunction(v0, v1, v2)
    // 这是整个三角形的有向面积的两倍 (即 Px = v2 时的结果)
    const float TotalArea_2X = edgeFunction(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y);

    // 如果三角形退化（面积为 0），则无法计算重心坐标
    if (std::abs(TotalArea_2X) < 1e-6) { // 1 * 10^{-6}，即 0.000001，Robust
        w0 = w1 = w2 = 0.0f;
        return 0.0f;
    }

    // w2 对应于 v2 对面的子三角形面积 (P, v0, v1)
    // w2 = Area(P, v0, v1) / Area(v0, v1, v2)
    const float w2_2X = edgeFunction(v0.x, v0.y, v1.x, v1.y, Px, Py);

    // w0 对应于 v0 对面的子三角形面积 (P, v1, v2)
    // w0 = Area(P, v1, v2) / Area(v0, v1, v2)
    const float w0_2X = edgeFunction(v1.x, v1.y, v2.x, v2.y, Px, Py);

    // w1 对应于 v1 对面的子三角形面积 (P, v2, v0)
    // w1_2X 可以通过 w0_2X + w1_2X + w2_2X = TotalArea_2X 间接求出，但直接计算更直观
    const float w1_2X = edgeFunction(v2.x, v2.y, v0.x, v0.y, Px, Py);

    // 归一化：将子三角形的有向面积比上总面积，得到重心坐标
    // 这种方法避免了昂贵的除法操作，只在最后归一化一次
    const float inverseTotalArea = 1.0f / TotalArea_2X;
    
    w0 = w0_2X * inverseTotalArea;
    w1 = w1_2X * inverseTotalArea;
    w2 = w2_2X * inverseTotalArea;

    return TotalArea_2X; // 返回 TotalArea_2X 以备后续使用，例如透视校正插值
}

void Rasterizer::drawTriangle(FrameBuffer &fb, const Vertex &v0, const Vertex &v1, const Vertex &v2, const YUVTexture &texture) {
    // 1. 计算三角形的包围盒
    // 确定 x 轴的最小和最大边界，并钳制在 FrameBuffer 范围内 [0, width-1]
    int minX = static_cast<int>(std::floor(std::min({v0.x, v1.x, v2.x})));
    int maxX = static_cast<int>(std::ceil(std::max({v0.x, v1.x, v2.x})));
    
    // 确定 y 轴的最小和最大边界，并钳制在 FrameBuffer 范围内 [0, height-1]
    int minY = static_cast<int>(std::floor(std::min({v0.y, v1.y, v2.y})));
    int maxY = static_cast<int>(std::ceil(std::max({v0.y, v1.y, v2.y})));
    
    // 确保包围盒不会超出 FrameBuffer 的边界
    minX = std::max(0, minX);
    maxX = std::min(fb.getWidth() - 1, maxX);
    minY = std::max(0, minY);
    maxY = std::min(fb.getHeight() - 1, maxY);
    
    // 2. 遍历三角形包围盒
    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            // 3. 点在三角形内的测试（重心坐标法或边函数法）
            
            // 计算重心坐标或边函数，判断点 (x, y) 是否在三角形内
            // 如果在三角形内，进行插值计算纹理坐标
//            if (在三角形内)
//            {
//                // 插值纹理坐标
//                float u = 插值(u0, u1, u2);
//                float v = 插值(v0, v1, v2);
//                
//                // 采样YUV
//                unsigned char y, u_val, v_val;
//                texture.sampleYUV(u, v, y, u_val, v_val);
//                
//                // 颜色空间转换
//                Color rgb = ColorSpace::yuvToRGB(y, u_val, v_val);
//
//                // 输出到帧缓冲
//                fb.setPixel(x, y, rgb);
//            }
        }
    }
}

} // namespace SoftRenderer
