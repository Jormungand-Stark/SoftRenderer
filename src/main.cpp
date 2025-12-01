//
//  main.cpp
//  softrender
//
//  Created by 封睿文 on 2025/11/19.
//

#include <core/FrameBuffer.hpp>
#include <texture/YUVTexture.hpp>
#include <rasterization/Rasterizer.hpp>
#include <iostream>

int main() {
    /**
     YUVTexture (YUV数据)
          ↓
     纹理采样 → 得到YUV值
          ↓
     色彩空间转换 → YUV→RGB
          ↓
     FrameBuffer (写入RGB结果)
          ↓
     保存为图片/显示
     */
    try {
        // 1. 创建帧缓冲作为输出目标
        SoftRenderer::FrameBuffer fb(800, 600);
        fb.clear();
        
        // 2. 加载YUV纹理
        std::string filename = "/Users/jormungand/Downloads/output.yuv";
        SoftRenderer::YUVTexture texture(filename, 1920, 1080);
        
        // 3. 光栅化
        SoftRenderer::Rasterizer rasterizer;
        
        // 4. 创建全屏四边形顶点
        std::vector<Vertex> quad = {
            // 屏幕坐标   // 纹理坐标
            // 左下三角形
            {0.0f, 0.0f, 0.0f, 0.0f},
            {800.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 600.0f, 0.0f, 1.0f},
            // 右上三角形
            {0.0f, 600.0f, 0.0f, 1.0f},
            {800.0f, 600.0f, 1.0f, 1.0f},
            {800.0f, 0.0f, 1.0f, 0.0f}
        };
        
        // 5. 渲染两个三角形
        rasterizer.drawTexturedTriangle(fb, quad[0], quad[1], quad[2], texture);
        rasterizer.drawTexturedTriangle(fb, quad[3], quad[4], quad[5], texture);
        
        // 绘制纯色三角形
//        rasterizer.drawSolidTriangle(fb, quad[0], quad[1], quad[2], {255, 0, 0});
//        rasterizer.drawSolidTriangle(fb, quad[3], quad[4], quad[5], {0, 255, 0});
        
        // 6. 保存结果
        fb.saveToPPM("/Users/jormungand/Downloads/output.ppm");
        std::cout << "渲染完成！保存为 output.ppm" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
    }
    std::cout << "Soft Renderer Started!" << std::endl;
    return 0;
}
