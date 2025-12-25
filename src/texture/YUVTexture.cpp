//
//  YUVTexture.cpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#include "YUVTexture.hpp"
#include <iostream>

namespace SoftRenderer {

    /// I420格式（YUV420P）的数据排列：[YYYYYYYY][UUUU][VVVV]
    /// Y平面：完整分辨率（width × height）
    /// U平面：1/4分辨率（width/2 × height/2）
    /// V平面：1/4分辨率（width/2 × height/2）
    /// 文件大小计算：width × height × 1.5 bytes
    YUVTexture::YUVTexture(const std::string &filename, int w, int h) : width_(w), height_(h) {
        if (width_ <= 0 || height_ <= 0)
        {
            throw std::invalid_argument("纹理尺寸必须大于0");
        }

        if (width_ % 2 != 0 || height_ % 2 != 0)
        {
            throw std::invalid_argument("YUV420要求宽高为偶数");
        }
        
        int y_size = w * h;
        int uv_size = (w / 2) * (h / 2);

        y_plane_.resize(y_size);
        u_plane_.resize(uv_size);
        v_plane_.resize(uv_size);

        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs)
        { // 等同于 if (file.fail() || file.bad())
            throw std::runtime_error("Failed to open YUV file: " + filename);
        }
        
        // check file size
        ifs.seekg(0, std::ios::end);    // 移到末尾
        size_t file_size = ifs.tellg(); // 计算size
        ifs.seekg(0, std::ios::beg);    // 完毕移至开头，便于后续读取操作。
        
        size_t expected_size = y_size + uv_size * 2;
        if (file_size < expected_size)
        {
            throw std::runtime_error("YUV文件大小不足: 期望 " +
                                     std::to_string(expected_size) + " bytes, 实际 " +
                                     std::to_string(file_size) + " bytes");
        }

        if (!ifs.read(reinterpret_cast<char *>(y_plane_.data()), y_size))
        {
            throw std::runtime_error("读取Y分量失败");
        }
        if (!ifs.read(reinterpret_cast<char *>(u_plane_.data()), uv_size))
        {
            throw std::runtime_error("读取U分量失败");
        }
        if (!ifs.read(reinterpret_cast<char *>(v_plane_.data()), uv_size))
        {
            throw std::runtime_error("读取V分量失败");
        }
    }

    void YUVTexture::sampleYUV(float u, float v,
                               unsigned char &y_val, unsigned char &u_val, unsigned char &v_val) const {
        switch (filter_mode_)
        {
        case TextureFilter::NEAREST:
            sampleNearest(u, v, y_val, u_val, v_val);
            break;
        case TextureFilter::BILINEAR:
            sampleBilinear(u, v, y_val, u_val, v_val);
            break;
        default:
            throw std::runtime_error("未知的纹理过滤模式");
            break;
        }
    }

    void YUVTexture::sampleNearest(float u, float v,
                                   unsigned char &y_val, unsigned char &u_val, unsigned char &v_val) const {

        // 1. 坐标转换，最邻近采样：四舍五入到最近的 “整数像素坐标”
        // uv是归一化之后的值，所以可以通过uv与图像宽高相乘获得像素索引
        // 注意：u * width_ 的范围是 [0.0, width_]，包含右边界
        // static_cast<int> 截断小数，得到范围 [0, width_]
        int pix_x = static_cast<int>(u * width_);  // 最邻近采样会直接取整舍弃小数部分，直接决定“用哪个像素”！！！
        int pix_y = static_cast<int>(v * height_); // “pix” 强调像素索引

        // 2. 边界钳位（CLAMP_TO_EDGE），纹理寻址模式/纹理环绕模式（Texture Wrap Mode）之一。
        // 处理 u=1.0 时 pix_x=width_ 的越界情况，以及 u<0.0 时的负值情况。
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
        y_val = y_plane_[y_index];

        // 4. U/V 分量采样 (4:2:0 降采样)
        int uv_x = pix_x / 2;
        int uv_y = pix_y / 2;
        int uv_width = width_ / 2;
        int uv_index = uv_y * uv_width + uv_x;

        u_val = u_plane_[uv_index];
        v_val = v_plane_[uv_index];
    }

    // 辅助函数：在指定平面上进行双线性采样
    float YUVTexture::samplePlaneBilinear(const std::vector<unsigned char>& plane,
                                          int planeWidth, int planeHeight,
                                          float u, float v) const {
        // 1. 坐标转换，双线性采样：downcast到最近的四个像素坐标
        // uv是归一化之后的值，所以可以通过uv与图像宽高相乘获得像素索引，将坐标转换到YUV平面（UV平面分辨率是Y平面的1/2）。
        /// NOTE: 注意不同于三角形包围盒中的”屏幕像素坐标“，这里是”纹理像素坐标“。
        float tex_x = u * planeWidth;  // UV平面宽度 = width_/2，“tex” 强调纹理空间坐标
        float tex_y = v * planeHeight; // UV平面高度 = height_/2

        // 2. 转换为基于像素中心的坐标系，将连续的“几何坐标”对齐回以“整数为中心”的插值系统。
        /**
         * 双线性过滤需要基于纹素中心计算权重，因此需要坐标转换：
         * 1. u ∈ [0,1] 是归一化纹理坐标；
         * 2. u * width ∈ [0, width] 映射到纹理像素范围；
         * 3. -0.5 将坐标系原点从像素左下角移到像素中心；
         * 
         * 为什么是 0.5 呢？
         * 1. 像素 n 覆盖的 u 范围是：[n / width, (n + 1) / width]
         * 中心点 u 值 = 范围中点 = (n / width + (n + 1) / width) / 2 = (2n + 1) / (2 width)
         * 2. 令 centerBasedX = n（基于像素 n 中心的坐标）
         * 带入上式： 
         * u = (2 * centerBasedX + 1) / (2 * width)
         * 2 * centerBasedX + 1 = 2 * width * u
         * 2 * centerBasedX = 2 * width * u - 1
         * centerBasedX = width * u - 0.5
         */
        float center_based_x = tex_x - 0.5f; // 水平方向
        float center_based_y = tex_y - 0.5f; // 垂直方向

        // 3. 确定参与插值的四个像素坐标
        int x0 = static_cast<int>(std::floor(center_based_x));
        int x1 = x0 + 1; // 右边像素索引,注意：x1 可能等于 width_，后续需要钳位处理
        int y0 = static_cast<int>(std::floor(center_based_y));
        int y1 = y0 + 1; // 上边像素索引,注意：y1 可能等于 height_，后续需要钳位处理

        // 4. 边界钳位（CLAMP_TO_EDGE）
        x0 = std::clamp(x0, 0, planeWidth - 1);
        x1 = std::clamp(x1, 0, planeWidth - 1);
        y0 = std::clamp(y0, 0, planeHeight - 1);
        y1 = std::clamp(y1, 0, planeHeight - 1);

        // 5. 计算插值权重系数 (s, t) 在 [0, 1) 之间
        // 这是centerBasedX的小数部分，表示离左边像素中心有多远
        float s = center_based_x - static_cast<float>(x0); // 水平权重
        float t = center_based_y - static_cast<float>(y0); // 垂直权重
        
        // 使用平滑函数收缩模糊带
        // 如果你想更“硬”（接近最近邻），可以多次迭代 smoothstep
        auto sharpen = [](float x) {
            return x * x * (3.0f - 2.0f * x); // 标准 smoothstep
        };

        s = sharpen(s);
        t = sharpen(t);

        s = std::clamp(s, 0.0f, 1.0f);
        t = std::clamp(t, 0.0f, 1.0f);

        // 6. 双线性插值采样：获取四个像素的U值。
        unsigned char u00 = plane[y0 * planeWidth + x0]; // 左下 (x0, y0)
        unsigned char u10 = plane[y0 * planeWidth + x1]; // 右下 (x1, y0)
        unsigned char u01 = plane[y1 * planeWidth + x0]; // 左上 (x0, y1)
        unsigned char u11 = plane[y1 * planeWidth + x1]; // 右上 (x1, y1)

        // 7. 双线性插值：先水平、后垂直。
        float bottom = (1.0f - s) * static_cast<float>(u00) + s * static_cast<float>(u10); // 下边插值
        float top = (1.0f - s) * static_cast<float>(u01) + s * static_cast<float>(u11);    // 上边插值
        float interpolated = (1.0f - t) * bottom + t * top;                                // 垂直插值
        return interpolated;
    }

    void YUVTexture::sampleBilinear(float u, float v, unsigned char &y_val, unsigned char &u_val, unsigned char &v_val) const {
        // 1. Y分量双线性插值采样（全分辨率）
        float y_interpolated = samplePlaneBilinear(y_plane_, width_, height_, u, v);
        y_val = static_cast<unsigned char>(std::clamp(y_interpolated, 0.0f, 255.0f));
        
        // 2. U分量双线性插值采样， (4:2:0 降采样)
        float u_interpolated = samplePlaneBilinear(u_plane_, width_ / 2, height_ / 2, u, v);
        u_val = static_cast<unsigned char>(std::clamp(u_interpolated, 0.0f, 255.0f));

        // 3. V分量双线性插值采样， (4:2:0 降采样)
        float v_interpolated = samplePlaneBilinear(v_plane_, width_ / 2, height_ / 2, u, v);
        v_val = static_cast<unsigned char>(std::clamp(v_interpolated, 0.0f, 255.0f));
    }

} // namespace SoftRenderer
