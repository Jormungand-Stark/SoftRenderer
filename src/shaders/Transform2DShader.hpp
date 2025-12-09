//
//  Transform2DShader.hpp
//  SoftRenderer
//
//  Created by Jormungand on 2025/12/8.
//

#ifndef Transform2DShader_hpp
#define Transform2DShader_hpp

#include "VertexShader.hpp"
#include <cmath>

namespace SoftRenderer {

    // 2D变换着色器的统一变量结构体
    struct Transform2DUniforms : public ShaderUniforms {
        float translateX = 0.0f;   // X方向平移
        float translateY = 0.0f;   // Y方向平移
        float scaleX = 1.0f;       // X方向缩放
        float scaleY = 1.0f;       // Y方向缩放
        float rotate_angle = 0.0f; // 旋转角度（弧度）

        bool isValid() const {
            return scaleX > 0.0f && scaleY > 0.0f;
        }
    };

    // 实现2D变换的顶点着色器
    class Transform2DShader : public VertexShader {
    public:
        void setUniforms(const Transform2DUniforms& uniforms);

        const Transform2DUniforms& getUniforms() const { return uniforms_; }

        virtual Vertex processVertex(const Vertex& in_vertex) override;
    private:
        Transform2DUniforms uniforms_;
    };

} // namespace SoftRenderer

#endif /* Transform2DShader_hpp */
