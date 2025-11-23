//
//  ColorSpace.hpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#ifndef ColorSpace_hpp
#define ColorSpace_hpp

#include <stdio.h>
#include "core/Color.hpp"

namespace SoftRenderer {
    enum class ColorSpaceStandard {
        BT601, // 标清电视标准
        BT709, // 高清
        BT2020 // 超高清
    };

    /**
     * 将YUV颜色空间转换为RGB颜色空间
     * @param y 亮度分量
     * @param u 色度分量U
     * @param v 色度分量V
     * @param standard 颜色空间标准
     * @return 转换后的RGB颜色
     */
    Color yuvToRGB(unsigned char y, unsigned char u, unsigned char v, ColorSpaceStandard standard = ColorSpaceStandard::BT601);

} // namespace SoftRenderer

#endif /* ColorSpace_hpp */
