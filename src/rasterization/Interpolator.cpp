//
//  Interpolator.cpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#include <algorithm>
#include "Interpolator.hpp"

namespace SoftRenderer {

void Interpolator::interpolateUV(float w0, float w1, float w2,
                               const Vertex& v0, const Vertex& v1, const Vertex& v2,
                               float& u, float& v) {
    // 简单线性插值，用点 P 离三个顶点的距离比例（w0, w1, w2），来混合三个顶点的纹理坐标。
    // P 点的 U 值，等于 V0 的 U 值乘以它所占的比例(w0)，加上 V1 的 U 值乘以它所占的比例(w1)，再加上 V2 的 U 值乘以它所占的比例(w2)。
    u = w0 * v0.u + w1 * v1.u + w2 * v2.u;
    v = w0 * v0.v + w1 * v1.v + w2 * v2.v;
    
    // 钳制 U, V 坐标在 [0, 1] 范围内，防止浮点误差导致越界。
    u = std::clamp(u, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);
}

} // namespace SoftRenderer
