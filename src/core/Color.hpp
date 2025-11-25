//
//  Color.hpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#ifndef Color_hpp
#define Color_hpp

#include <cstdint>

// 存储 RGB 三通道（每个通道 0-255）
struct Color {
    uint8_t r, g, b;

    Color() : r(0), g(0), b(0) {}
    Color(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
};

#endif /* Color_hpp */
