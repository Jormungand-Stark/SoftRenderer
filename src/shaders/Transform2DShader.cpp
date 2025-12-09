//
//  Transform2DShader.cpp
//  SoftRenderer
//
//  Created by Jormungand on 2025/12/8.
//

#include <stdexcept>

#include "Transform2DShader.hpp"

namespace SoftRenderer {
    void Transform2DShader::setUniforms(const Transform2DUniforms &uniforms) {
        if (!uniforms.isValid()) {
            throw std::invalid_argument("Invalid Transform2DUniforms: scale factors must be positive.");
        }
        uniforms_ = uniforms;
    }

    Vertex Transform2DShader::processVertex(const Vertex& in_vertex) {
        // 提取输入顶点位置
        float x = in_vertex.x;
        float y = in_vertex.y;

        /**
         * 变换顺序：缩放 (Scale) -> 旋转 (Rotate) -> 平移 (Translate)
         * 1. 如果先位移再缩放，位移的向量也会同样被缩放（向某方向移动2米，2米也许会被缩放成1米）!
         * 2. 如果在平移后旋转，物体会绕着“世界坐标系”的原点旋转（像行星绕太阳），而不是在原地自转。
         * 3. 因此将已完成所有“自身变换”的物体，从原点移动到最终的世界坐标位置。
         */

        // 应用缩放
        x *= uniforms_.scaleX;
        y *= uniforms_.scaleY;

        // 应用旋转（绕自身原点）
        float cos_theta = std::cos(uniforms_.rotate_angle);
        float sin_theta = std::sin(uniforms_.rotate_angle);
        float rotatedX = x * cos_theta - y * sin_theta;
        float rotatedY = x * sin_theta + y * cos_theta;

        // 应用平移
        rotatedX += uniforms_.translateX;
        rotatedY += uniforms_.translateY;

        // 创建输出顶点，保留其他属性不变
        Vertex out_vertex = in_vertex;
        out_vertex.x = rotatedX;
        out_vertex.y = rotatedY;

        return out_vertex;
    }
} // namespace SoftRenderer