//
//  FrameBuffer.cpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#include "FrameBuffer.hpp"

namespace SoftRenderer {

    void FrameBuffer::clear(const Color &clear_color) {
        std::fill(pixels.begin(), pixels.end(), clear_color);
    }

    bool FrameBuffer::saveToPPM(std::string filename) {
        std::ofstream ofs(filename, std::ios::binary);

        if (!ofs)
        {
            return false;
        }

        // PPM 文件头，由3部分组成，通过换行或空格进行分割，一般PPM的标准是空格。
        // 1. 标识符：一个两个字节的字符串，例如 "P3" 或 "P6"，表示文件类型和编码格式。
        // - ASCII (P3)：文本格式，易于阅读，但文件体积较大。
        // - Binary (P6)：二进制格式，效率更高，文件体积相对较小。
        // 2. 图像尺寸：宽度和高度的整数值，表示图像的像素尺寸。
        // 3. 最大颜色值：一个整数，通常为 255，表示每个颜色通道的最大值。
        ofs << "P6 " << width << " " << height << " 255\n"; // 最大颜色值 255

        // 写入像素数据，按行优先顺序存储，每个像素由3个字节表示（R、G、B）
        for (const auto &pixel : pixels)
        {
            // p6格式明确了接下来的像素数据将是二进制形式，因此数据之间无需分隔
            ofs << pixel.r << pixel.g << pixel.b;
        }

        return true;
    }

    void FrameBuffer::setPixel(int x, int y, const Color &color) {
        // 由于 unsigned char 天然就是 0 到 255（或 0 到 256 个值），因此无需对 color 做判断。
        if (x >= 0 && x <= width && y >= 0 && y <= height)
        {
            int index = y * width + x;
            pixels[index] = color;
        }
    }

    Color FrameBuffer::getPixel(int x, int y) const {
        int index = y * width + x;
        return pixels[index];
    }

} // namespace SoftRenderer