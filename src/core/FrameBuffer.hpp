//
//  FrameBuffer.hpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#ifndef FrameBuffer_hpp
#define FrameBuffer_hpp

#include <vector>
#include "Color.hpp"

struct FrameBuffer {
    int width, height;              // 分辨率
    std::vector<Color> pixels;      // 存储渲染结果，像素数组，大小 = width * height
    
    // 需要哪些方法？
    // 1. 清屏
    // 2. 设置像素
    // 3. 保存为图片
    // 4. 其他？

    FrameBuffer(int w, int h) : width(w), height(h), pixels(w * h) {}

    // 清屏函数，把整个帧缓冲填充为指定颜色
    

    // 设置某个像素点的颜色

    // 保存帧缓冲为 PPM 图片文件（简单的 RGB 格式）
};

#endif /* FrameBuffer_hpp */
