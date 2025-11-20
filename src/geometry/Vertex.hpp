//
//  Vertex.hpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#ifndef Vertex_hpp
#define Vertex_hpp

#include <stdio.h>

// 顶点结构体，包含二维坐标和颜色
struct Vertex {
    float x, y;   // 顶点坐标（屏幕空间）
    float u, v;   // 纹理坐标
#warning 在纹理渲染中可能不需要顶点颜色 color，因为颜色来自纹理采样。
    // Color color;  // 顶点颜色

    Vertex(float x = 0, float y = 0, float ucoord = 0, float vcoord = 0)
        : x(x), y(y), u(ucoord), v(vcoord) {}
};

#endif /* Vertex_hpp */
