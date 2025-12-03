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
    /**
     * - UV 坐标 (u, v)： 纹理坐标永远是 0.0 ~ 1.0 之间的浮点数，需要在三角形内部进行浮点插值。
     * - 屏幕坐标 (x, y)： 即使目标是像素坐标（整数），在光栅化过程中，计算三角形的边缘、重心坐标，
     * 以及 亚像素精度（Sub-pixel Accuracy） 都需要浮点数。如果使用整数，会丢失精度，导致锯齿严重。
     * 因此必须为浮点数。
     */
    float x, y;   // 顶点坐标（屏幕空间）
    float u, v;   // 纹理坐标
    // NOTE: 在纹理渲染中可能不需要顶点颜色 color，因为颜色来自纹理采样。
    // Color color;  // 顶点颜色

    Vertex(float x = 0, float y = 0, float ucoord = 0, float vcoord = 0)
        : x(x), y(y), u(ucoord), v(vcoord) {}
};

#endif /* Vertex_hpp */
