//
//  Rasterizer.cpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#include <cmath>
#include <algorithm>
#include "ColorSpace.hpp"
#include "Rasterizer.hpp"
#include "Interpolator.hpp"

namespace SoftRenderer {

void Rasterizer::drawTexturedTriangle(FrameBuffer &fb, const Vertex &v0, const Vertex &v1, const Vertex &v2, const YUVTexture &texture) {
    // 1. 计算三角形的包围盒
    // 确定 x 轴的最小和最大边界，并钳制在 FrameBuffer 范围内 [0, width-1]。
    int minX = static_cast<int>(std::floor(std::min({v0.x, v1.x, v2.x})));
    int maxX = static_cast<int>(std::ceil(std::max({v0.x, v1.x, v2.x})));
    
    // 确定 y 轴的最小和最大边界，并钳制在 FrameBuffer 范围内 [0, height-1]。
    int minY = static_cast<int>(std::floor(std::min({v0.y, v1.y, v2.y})));
    int maxY = static_cast<int>(std::ceil(std::max({v0.y, v1.y, v2.y})));
    
    // 确保包围盒不会超出 FrameBuffer 的边界
    minX = std::max(0, minX);
    maxX = std::min(fb.getWidth() - 1, maxX);
    minY = std::max(0, minY);
    maxY = std::min(fb.getHeight() - 1, maxY);
    
    // 2. 遍历三角形包围盒内的每个像素 (x,y)，将像素索引转换为几何采样点（px，py），依赖于 v0、v1、v2 坐标。
    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            // 像素是1x1的方格区域，不是数学上的点。
            // 如果用 (x，y)，它同时是四个像素的角点，系统很难确定这个像素是否应该被覆盖。
            // 因此，用像素中心 (x+0.5, y+0.5) 作为采样点。
            // 这是图形学中的标准约定，能确保每个像素只被判断一次，并得到最准确的颜色覆盖。
            float px = static_cast<float>(x) + 0.5f;
            float py = static_cast<float>(y) + 0.5f;
            
            // 3. 计算重心坐标
            float w0, w1, w2;
            // 如果三角形退化（computeBarycentric 返回的面积接近0），跳过该像素的处理
            auto area = computeBarycentric(px, py, v0, v1, v2, w0, w1, w2);
            // 几何判断，基于非负性判断像素（px，py）是否在三角形内。
            // if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
            if (area != 0 && w0 >= -1e-5f && w1 >= -1e-5f && w2 >= -1e-5f) { // 使用微小容差
                // 4. 属性插值，依赖重心坐标 (w0, w1, w2) 计算像素对应的纹理坐标 (u，v)
                float u, v;
                Interpolator::interpolateUV(w0, w1, w2, v0, v1, v2, u, v);
                
                // 5. 纹理采样，从 YUV 纹理中获取颜色数据，依赖于插值后的 (u，v) 坐标。
                unsigned char y_val, u_val, v_val;
                texture.sampleYUV(u, v, y_val, u_val, v_val);
                
                // 6. 颜色空间转换，将 YUV 转换为可显示的 RGB，依赖于采样的 Y, U, V 值。
                Color rgb = yuvToRGB(y_val, u_val, v_val);
                
                // 7. 将最终颜色写入帧缓冲
                fb.setPixel(x, y, rgb);
            }
        }
    }
}

/**
 * @brief
 * 1. 该函数计算以 P0 为起点，分别指向 P1 和 P2 的两个向量 P0P1 和 P0P2
 * 所围成的平行四边形的有向面积。该值等于对应三角形 (P0, P1, P2) / (v0, v1, v2) 面积的两倍。
 * 2. 有向面积的正负号可用于判断点的内外关系（例如光栅化优化）：
 *      - 如果三个叉乘的结果符号相同，则点 P 在三角形内部。
 *      - 如果存在任何一个叉乘结果为负，则点 P 在三角形外部。
 *      - 如果任何一个叉乘结果为零，则点 P 在三角形的某条边上。
 *
 * @param x0 P0点的x坐标
 * @param y0 P0点的y坐标
 * @param x1 P1点的x坐标
 * @param y1 P1点的y坐标
 * @param x2 P2点的x坐标
 * @param y2 P2点的y坐标
 * @return 浮点数，表示有向面积的两倍。
 */
inline float edgeFunction(float x0, float y0, float x1, float y1, float x2, float y2) {
    // 隐式构建向量 A (P0 -> P1): Ax = (x1 - x0), Ay = (y1 - y0)
    // 隐式构建向量 B (P0 -> P2): Bx = (x2 - x0), By = (y2 - y0)
    
    // 二维向量叉积公式 (A_x * B_y - A_y * B_x):
    // (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0)
    return (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
}

float Rasterizer::computeBarycentric(float px, float py,
                                     const Vertex& v0, const Vertex& v1, const Vertex& v2,
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
    const float w2_2X = edgeFunction(v0.x, v0.y, v1.x, v1.y, px, py);

    // w0 对应于 v0 对面的子三角形面积 (P, v1, v2)
    // w0 = Area(P, v1, v2) / Area(v0, v1, v2)
    const float w0_2X = edgeFunction(v1.x, v1.y, v2.x, v2.y, px, py);

    // w1 对应于 v1 对面的子三角形面积 (P, v2, v0)
    // w1_2X 可以通过 w0_2X + w1_2X + w2_2X = TotalArea_2X 间接求出，但直接计算更直观
    const float w1_2X = edgeFunction(v2.x, v2.y, v0.x, v0.y, px, py);

    // 归一化：将子三角形的有向面积比上总面积，得到重心坐标
    // 这种方法避免了昂贵的除法操作，只在最后归一化一次
    const float inverseTotalArea = 1.0f / TotalArea_2X;
    
    w0 = w0_2X * inverseTotalArea;
    w1 = w1_2X * inverseTotalArea;
    w2 = w2_2X * inverseTotalArea;

    return TotalArea_2X; // 返回 TotalArea_2X 以备后续使用，例如透视校正插值
}

} // namespace SoftRenderer
