//
//  FrameBuffer.hpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#ifndef FrameBuffer_hpp
#define FrameBuffer_hpp

#include <vector>
#include <string>
#include <fstream>
#include "Color.hpp"

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
// 帧缓冲，用来存储屏幕上的像素数据，数据输出层，存储和管理最终显示结果
// 存储光栅化、着色后的最终像素颜色，直接对应到屏幕像素位置，工作在RGB色彩空间，可以直接显示。
namespace SoftRenderer {
     class FrameBuffer {
     public:
          FrameBuffer(int w, int h) : width(w), height(h), pixels(w * h) {}

          // 清屏函数，把整个帧缓冲填充为指定颜色
          void clear(const Color &clear_color = Color(0, 0, 0));

          // 保存帧缓冲为 PPM 图片文件（简单的 RGB 格式）
          bool saveToPPM(std::string filename);

          // 设置某个像素点的颜色
          void setPixel(int x, int y, const Color &color);
          Color getPixel(int x, int y) const;
          int getWidth() const { return width; }
          int getHeight() const { return height; }

     private:
          int width, height;         // 分辨率
          std::vector<Color> pixels; // 存储渲染结果，像素数组，大小 = width * height
     };

} // namespace SoftRenderer

#endif /* FrameBuffer_hpp */
