//
//  VertexShader.hpp
//  SoftRenderer
//
//  Created by Jormungand on 2025/12/8.
//

#ifndef VertexShader_hpp
#define VertexShader_hpp

#include <geometry/Vertex.hpp>

namespace SoftRenderer {

    // Uniforms 结构体（如果需要传递统一变量，可以在这里添加成员）
    struct ShaderUniforms {
        // 例如：变换矩阵、光照参数等
        // glm::mat4 modelViewProjectionMatrix;
    };

    class VertexShader {
    public:
        virtual ~VertexShader() = default;

        // 处理单个顶点的纯虚函数
        virtual Vertex processVertex(const Vertex& in_vertex) = 0;

        // 可选：批量处理多个顶点
        virtual void processVertices(Vertex* out_vertices, const Vertex* inVertices, size_t count) {
            for (size_t i = 0; i < count; ++i) {
                out_vertices[i] = processVertex(inVertices[i]);
            }
        }
    };

} // namespace SoftRenderer

#endif /* VertexShader_hpp */
