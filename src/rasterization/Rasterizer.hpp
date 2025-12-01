//
//  Rasterizer.hpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#ifndef Rasterizer_hpp
#define Rasterizer_hpp

#include <vector>
#include "geometry/Vertex.hpp"
#include "core/FrameBuffer.hpp"
#include "texture/YUVTexture.hpp"

// 光栅化
/**
 * YUVTexture (存储原始数据)
 *     ↓
 * Vertex几何 (定义三角形和纹理坐标)
 *     ↓
 * Rasterizer光栅化
 *     ├── 纹理采样 ← YUVTexture.sampleYUV()
 *     ├── 色彩空间转换 ← ColorSpace::yuvToRGB()
 *     └── 像素写入 ← FrameBuffer.setPixel()
 *     ↓
 * FrameBuffer (存储最终结果)
 */

namespace SoftRenderer {
    class Rasterizer {
    public:
        Rasterizer() = default;
        
        /**
         * 绘制三角形并进行纹理映射
         * @param fb 目标帧缓冲（包含尺寸），往哪里画
         * @param v0 三角形顶点0
         * @param v1 三角形顶点1
         * @param v2 三角形顶点2
         * @param texture 源纹理对象
         */
        void drawTexturedTriangle(FrameBuffer& fb,
                                  const Vertex& v0,
                                  const Vertex& v1,
                                  const Vertex& v2,
                                  const YUVTexture& texture);
        
        // 辅助方法：绘制纯色三角形（用于调试）
        void drawSolidTriangle(FrameBuffer& fb,
                               const Vertex& v0,
                               const Vertex& v1,
                               const Vertex& v2,
                               const Color& color);

    private:
        /**
         * 计算给定点 P(x, y) 的重心坐标
         * 方案：有向面积比法
         * 顶点 Vi 的重心坐标 wi 等于点 P 与其对边顶点 V(i+1), V(i+2) 构成的子三角形的有向面积，
         * 占整个三角形 V0, V1, V2 有向面积的比例。
         * 公式：`w_i = Area(P, V(i+1), V(i+2)) / Area(V0, V1, V2)`
         * 面积通过 2D 向量的叉积（即 Edge Function）计算，结果为三角形面积的两倍。
         * 子三角形的 edgeFunction 必须使用与总三角形相同的绕序，来确保分子分母符号一致。
         * @param px 待测试点的 X 坐标
         * @param py 待测试点的 Y 坐标
         * @param v0 三角形的第一个顶点
         * @param v1 三角形的第二个顶点
         * @param v2 三角形的第三个顶点
         * @param w0 用于返回的第一个重心坐标（输出参数）
         * @param w1 用于返回的第二个重心坐标（输出参数）
         * @param w2 用于返回的第三个重心坐标（输出参数）
         * @return 三角形的面积的两倍 (用于后续归一化)，如果面积为 0 则返回 0
         */
        float computeBarycentric(float px, float py,
                                 const Vertex& v0, const Vertex& v1, const Vertex& v2,
                                 float& w0, float& w1, float& w2);
    };

} // namespace SoftRenderer

#endif /* Rasterizer_hpp */
