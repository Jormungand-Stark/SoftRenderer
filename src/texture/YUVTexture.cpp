//
//  YUVTexture.cpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#include "YUVTexture.hpp"

namespace SoftRenderer {

    /// I420格式（YUV420P）的数据排列：[YYYYYYYY][UUUU][VVVV]
    /// Y平面：完整分辨率（width × height）
    /// U平面：1/4分辨率（width/2 × height/2）
    /// V平面：1/4分辨率（width/2 × height/2）
    /// 文件大小计算：width × height × 1.5 bytes
    YUVTexture::YUVTexture(const std::string &filename, int w, int h) : width_(w), height_(h) {
        if (width_ <= 0 || height_ <= 0) {
            throw std::invalid_argument("纹理尺寸必须大于0");
        }
        
        if (width_ % 2 != 0 || height_ % 2 != 0) {
            throw std::invalid_argument("YUV420要求宽高为偶数");
        }
        
        int y_size = w * h;
        int uv_size = (w / 2) * (h / 2);

        y_plane.resize(y_size);
        u_plane.resize(uv_size);
        v_plane.resize(uv_size);

        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs) { // 等同于 if (file.fail() || file.bad())
            throw std::runtime_error("Failed to open YUV file: " + filename);
        }
        
        // check file size
        ifs.seekg(0, std::ios::end);    // 移到末尾
        size_t file_size = ifs.tellg(); // 计算size
        ifs.seekg(0, std::ios::beg);    // 完毕移至开头，便于后续读取操作。
        
        size_t expected_size = y_size + uv_size * 2;
        if (file_size < expected_size) {
            throw std::runtime_error("YUV文件大小不足: 期望 " +
                                     std::to_string(expected_size) + " bytes, 实际 " +
                                     std::to_string(file_size) + " bytes");
        }

        if (!ifs.read(reinterpret_cast<char*>(y_plane.data()), y_size)) {
            throw std::runtime_error("读取Y分量失败");
        }
        if (!ifs.read(reinterpret_cast<char *>(u_plane.data()), uv_size)) {
            throw std::runtime_error("读取U分量失败");
        }
        if (!ifs.read(reinterpret_cast<char *>(v_plane.data()), uv_size)) {
            throw std::runtime_error("读取V分量失败");
        }
    }

    void YUVTexture::sampleYUV(float u, float v,
                               unsigned char &y_val, unsigned char &u_val, unsigned char &v_val) const {

        // 1. 坐标转换，最邻近采样：四舍五入到最近的整数像素坐标
        // uv是归一化之后的值，所以可以通过uv与图像宽高相乘获得像素索引
        int pix_x = static_cast<int>(u * width_);  // 水平方向，0 到 width - 1
        int pix_y = static_cast<int>(v * height_); // 垂直方向，0 到 height - 1

        // 2. 边界钳位（CLAMP_TO_EDGE），纹理寻址模式/纹理环绕模式（Texture Wrap Mode）之一。
        /**
         * 为什么不直接前期纹理坐标映射像素坐标的时候就将宽高钳位呢？就是直接 x = u * （width-1），y 同理。
         *
         * 双线性插值 (Bilinear Interpolation)： 在实际渲染中，采样通常不是最邻近（Nearest Neighbor），而是双线性插值。
         * 插值需要计算周围四个像素的权重，而标准做法的浮点坐标 x ∈ [0.0, width]  才能正确地表示采样点在像素之间的位置，
         * 从而计算出正确的插值权重。
         *
         * 重复寻址模式 (Repeat/Wrap)： 如果纹理寻址模式设置为重复，当 u=1.1 时，标准做法能很容易通过
         * 取模运算 (fmod(u, 1.0) × width) 得到正确的新坐标 0.1 × width。而使用 (width - 1)  的方法会使数学处理更复杂。
         * 因此，u * width 是基于浮点纹理空间的标准映射方式，配合后期钳位处理越界情况。
         */
        pix_x = pix_x < 0 ? 0 : pix_x;
        pix_x = pix_x >= width_ ? width_ - 1 : pix_x;

        pix_y = pix_y < 0 ? 0 : pix_y;
        pix_y = pix_y >= height_ ? height_ - 1 : pix_y;

        // 3. Y 分量采样
        int y_index = pix_y * width_ + pix_x;
        y_val = y_plane[y_index];

        // 4. U/V 分量采样 (4:2:0 降采样)
        int uv_x = pix_x / 2;
        int uv_y = pix_y / 2;
        int uv_width = width_ / 2;
        int uv_index = uv_y * uv_width + uv_x;

        u_val = u_plane[uv_index];
        v_val = v_plane[uv_index];
    }

} // namespace SoftRenderer
