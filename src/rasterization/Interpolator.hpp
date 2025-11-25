//
//  Interpolator.hpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#ifndef Interpolator_hpp
#define Interpolator_hpp

#include "geometry/Vertex.hpp"

// 插值器
namespace SoftRenderer {

class Interpolator {
public:
    /**
     * 使用重心坐标插值纹理坐标
     * 给三角形上色，颜色是从一张纹理上采样的得到，即已知一个点 P 在三角形中的位置比例，求这个点的纹理坐标 (U, V) 值。
     * 公式：UP = w0 * U0 + w1 * U1 + w2 * U2
     *      VP = w0 * V0 + w1 * V1 + w2 * V2
     * @param w0 P 点离 V0 多近（如果 w0 = 1，点就在 V0 上）。
     * @param w1 P 点离 V1 多近，即第二个重心坐标。
     * @param w2 P 点离 V2 多近，即第三个重心坐标。
     * @param v0 三角形的第一个顶点，其成员 v0.u, v0.v 包含了纹理坐标 (U0, V0)。
     * @param v1 三角形的第二个顶点，其成员 v1.u, v1.v 包含了纹理坐标 (U1, V1)。
     * @param v2 三角形的第三个顶点，其成员 v2.u, v2.v 包含了纹理坐标 (U2, V2)。
     * @param u 输出参数，插值后的 U 坐标。
     * @param v 输出参数，插值后的 V 坐标。
     */
    static void interpolateUV(float w0, float w1, float w2,
                       const Vertex& v0, const Vertex& v1, const Vertex& v2,
                       float& u, float& v);
};
    
} // namespace SoftRenderer

#endif /* Interpolator_hpp */
