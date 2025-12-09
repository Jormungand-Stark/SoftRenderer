//
//  PassThroughVertexShader.hpp
//  SoftRenderer
//
//  Created by Jormungand on 2025/12/8.
//

#ifndef PassThroughVertexShader_hpp
#define PassThroughVertexShader_hpp

#include "VertexShader.hpp"

namespace SoftRenderer {

    // 一个简单的顶点着色器实现，直接传递输入顶点
    class PassThroughVertexShader : public VertexShader {
    public:
        virtual Vertex processVertex(const Vertex& inVertex) override {
            // 直接返回输入顶点，不做任何变换，标志着“顶点处理阶段”在架构上已存在
            return inVertex;
        }
    };
} // namespace SoftRenderer

#endif /* PassThroughVertexShader_hpp */
