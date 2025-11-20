//
//  Color.hpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#ifndef Color_hpp
#define Color_hpp

#include <stdio.h>

// 存储 RGB 三通道（每个通道 0-255）
struct Color {
    unsigned char r, g, b;

    Color() : r(0), g(0), b(0) {}
    Color(unsigned char red, unsigned char green, unsigned char blue) : r(red), g(green), b(blue) {}
};


#endif /* Color_hpp */
