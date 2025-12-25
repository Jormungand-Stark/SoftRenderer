//
//  Rasterizer.cpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#include <cmath>
#include <algorithm>
#include "texture/ColorSpace.hpp"
#include "Rasterizer.hpp"
#include "Interpolator.hpp"

namespace SoftRenderer {

/**
 * @brief
 * 1. 该函数计算以 P0 为起点，分别指向 P1 和 P2 的两个向量 P0P1 和 P0P2
 *   所围成的平行四边形的有向面积。该值等于对应三角形 (P0, P1, P2) or  (v0, v1, v2) 面积的两倍。
 * 2. 有向面积的正负号可用于判断点的内外关系（例如光栅化优化）：
 *      - 如果三个叉乘的结果符号相同，则点 P 在三角形内部。
 *      - 如果存在任何一个叉乘结果为负，则点 P 在三角形外部。
 *      - 如果任何一个叉乘结果为零，则点 P 在三角形的某条边上。
 *
 * @param ax 向量起始点 A 的 x 坐标
 * @param ay 向量起始点 A 的 y 坐标
 * @param bx 向量终点 B 的 x 坐标
 * @param by 向量终点 B 的 y 坐标
 * @param cx 向量终点 C 的 x 坐标
 * @param cy 向量终点 C 的 y 坐标
 * @return 浮点数，有向面积的两倍。符号表示 C 相对于 A->B 的方向。
 */
inline float edgeFunction(float ax, float ay, float bx, float by, float cx, float cy) {
    // 向量 V_AB : V_AB.x = (bx - ax), V_AB.y = (by - ay), V_AB = (bx - ax, by - ay)
    // 向量 V_AC : V_AC.x = (cx - ax), V_AC.y = (cy - ay), V_AC = (cx - ax, cy - ay)
    
    // 二维向量叉积公式 V_AB.x * V_AC.y - V_AB.y * V_AC.x:
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

void Rasterizer::drawTexturedTriangle(FrameBuffer &fb, const Vertex &v0, const Vertex &v1, const Vertex &v2, const YUVTexture &texture) {
    // 1. 预计算三角形的总面积和倒数（两倍有向面积用于重心坐标归一化，倒数可以避免重复计算）
    const float total_area_2X = edgeFunction(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y);

    // 如果三角形退化（面积为 0），直接跳过渲染
    if (std::abs(total_area_2X) < 1e-6) {
        return;
    }

    const float invtotal_area_2X = 1.0f / total_area_2X;  // 预先计算倒数，变除为乘

    // 2. 为加速子面积计算，预存顶点坐标差值
    // 这里计算的是调用 edgeFunction 时，那些不依赖于 (px, py) 的部分。
    // 对于 edgeFunction(v1, v2, P)，其公式为: (v2.x - v1.x) * (py - v1.y) - (v2.y - v1.y) * (px-v1.x)
    // 把其中不含P的、只和顶点有关的部分提出来，可以重写为: A0 * py - B0 * px + C0，其中：
    // A0 = (v2.x - v1.x),  B1 = (v2.y - v1.y), C1 = -(A1 * v1.y - B1 * v1.x)
    float A_for_w0 = v2.x - v1.x;
    float B_for_w0 = v2.y - v1.y;
    float C_for_w0 = -(A_for_w0 * v1.y - B_for_w0 * v1.x);

    // w1 对应 edgeFunction(v2, v0, P) = A1 * py - B1 * px + C1
    float A_for_w1 = v0.x - v2.x;
    float B_for_w1 = v0.y - v2.y;
    float C_for_w1 = -(A_for_w1 * v2.y - B_for_w1 * v2.x);

    // w2 对应 edgeFunction(v0, v1, P)，但根据 w2 = 1 - w0 - w1，可以省去。

    // 3. 计算三角形的包围盒
    // v0.x, v1.x, v2.x诚然代表三角形在无限精确的笛卡尔坐标系中的顶点位置，但强制类型转换会将他们从“几何世界”映射到
    // “屏幕像素阵列”，量化成像素整数索引，确定 x 轴的最小和最大边界，并钳制在 FrameBuffer 范围内 [0, width-1]。
    int min_x = static_cast<int>(std::floor(std::min({v0.x, v1.x, v2.x})));
    int max_x = static_cast<int>(std::ceil(std::max({v0.x, v1.x, v2.x})));
    
    // 确定 y 轴的最小和最大边界，并钳制在 FrameBuffer 范围内 [0, height-1]。
    int min_y = static_cast<int>(std::floor(std::min({v0.y, v1.y, v2.y})));
    int max_y = static_cast<int>(std::ceil(std::max({v0.y, v1.y, v2.y})));
    
    // 确保包围盒不会超出 FrameBuffer 的边界
    min_x = std::max(0, min_x);
    max_x = std::min(fb.getWidth() - 1, max_x);
    min_y = std::max(0, min_y);
    max_y = std::min(fb.getHeight() - 1, max_y);
    
    // 4. 遍历三角形包围盒内的每个像素 (x,y)，将像素索引转换为几何采样点（px，py），依赖于 v0、v1、v2 坐标。
    // 在内存访问上，按行访问（y在外层）通常对 CPU 缓存（Cache）更友好。
    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            // 像素是 1x1 的方格区域，不是数学上的点。
            // 如果用 (x, y)，它同时是四个像素的角点，系统很难确定这个像素是否应该被覆盖。
            // 即像素 (x, y) 的覆盖区域是 [x, x+1] × [y, y+1]。
            // 为了避免这种歧义，图形学中通常采用像素中心 (x+0.5, y+0.5) 作为采样点。
            // 能确保每个像素只被判断一次，并得到最准确的颜色覆盖。是从离散的“格子编号”进入连续的“几何空间”。
            float px = static_cast<float>(x) + 0.5f;
            float py = static_cast<float>(y) + 0.5f;

            // 5. 计算重心坐标
            // 利用预计算的系数，快速算出子面积，等价于调用 edgeFunction，但省去了大量重复的顶点坐标减法。
            float area_w0 = A_for_w0 * py - B_for_w0 * px + C_for_w0;
            float area_w1 = A_for_w1 * py - B_for_w1 * px + C_for_w1;

            // 归一化
            float w0 = area_w0 * invtotal_area_2X;
            float w1 = area_w1 * invtotal_area_2X;
            float w2 = 1.0f - w0 - w1; // 利用重心坐标和为1的性质，省去第三次 edgeFunction 调用！
            
            // 几何判断，基于非负性判断像素（px，py）是否在三角形内。
            // 使用微小容差，等价于 if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
            if (w0 >= -1e-5f && w1 >= -1e-5f && w2 >= -1e-5f) {
                // 6. 属性插值，依赖重心坐标 (w0, w1, w2) 计算像素对应的纹理坐标 (u，v)
                float u, v;
                Interpolator::interpolateUV(w0, w1, w2, v0, v1, v2, u, v);
                
                // 7. 纹理采样，从 YUV 纹理中获取颜色数据，依赖于插值后的 (u，v) 坐标。
                unsigned char y_val, u_val, v_val;
                texture.sampleYUV(u, v, y_val, u_val, v_val);
                
                // 8. 颜色空间转换，将 YUV 转换为可显示的 RGB，依赖于采样的 Y, U, V 值。
                Color rgb = yuvToRGB(y_val, u_val, v_val);
                
                // 9. 将最终颜色写入帧缓冲
                fb.setPixel(x, y, rgb);
            }
        }
    }
}

void Rasterizer::drawSolidTriangle(FrameBuffer& fb,
                                   const Vertex& v0,
                                   const Vertex& v1,
                                   const Vertex& v2,
                                   const Color& color) {
    // 复用 drawTexturedTriangle 中的预计算优化逻辑
    const float total_area_2X = edgeFunction(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y);
    if (std::abs(total_area_2X) < 1e-6) return;
    
    const float inv_total_area_2X = 1.0f / total_area_2X;
    
    // 预计算系数（与drawTexturedTriangle相同）
    float A_for_w0 = v2.x - v1.x;
    float B_for_w0 = v2.y - v1.y;
    float C_for_w0 = -(A_for_w0 * v1.y - B_for_w0 * v1.x);
    
    float A_for_w1 = v0.x - v2.x;
    float B_for_w1 = v0.y - v2.y;
    float C_for_w1 = -(A_for_w1 * v2.y - B_for_w1 * v2.x);
    
    // 包围盒计算
    int min_x = static_cast<int>(std::floor(std::min({v0.x, v1.x, v2.x})));
    int max_x = static_cast<int>(std::ceil(std::max({v0.x, v1.x, v2.x})));
    int min_y = static_cast<int>(std::floor(std::min({v0.y, v1.y, v2.y})));
    int max_y = static_cast<int>(std::ceil(std::max({v0.y, v1.y, v2.y})));
    
    // 钳制
    min_x = std::max(0, min_x);
    max_x = std::min(max_x, fb.getWidth() - 1);
    min_y = std::max(0, min_y);
    max_y = std::min(max_y, fb.getHeight() - 1);
    
    // 光栅化，像素坐标 -> 纹理坐标
    for (int x = min_x; x <= max_x; ++x) {
        for (int y = min_y; y <= max_y; ++y) {
            // 取像素中心作为像素块唯一代表
            float px = static_cast<float>(x) + 0.5f;
            float py = static_cast<float>(y) + 0.5f;
            
            // 使用优化版重心坐标计算
            float area_w0 = A_for_w0 * py - B_for_w0 * px + C_for_w0;
            float area_w1 = A_for_w1 * py - B_for_w1 * px + C_for_w1;
            
            float w0 = area_w0 * inv_total_area_2X;
            float w1 = area_w1 * inv_total_area_2X;
            float w2 = 1.0f - w0 - w1;
            
            if (w0 >= -1e-5f && w1 >= -1e-5f && w2 >= -1e-5f) {
                fb.setPixel(x, y, color);
            }
        }
    }
}

} // namespace SoftRenderer
