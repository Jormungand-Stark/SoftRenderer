//
//  YUVTexture.hpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#ifndef YUVTexture_hpp
#define YUVTexture_hpp

#include <vector>
#include <string>
#include <fstream>

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
// 数据输入层，存储和管理原始图像数据，
// 为渲染管线提供纹理采样服务，工作在YUV色彩空间，需要转换才能显示。
namespace SoftRenderer {
    class YUVTexture {
    public:
        // 从文件加载数据
        YUVTexture(const std::string &filename, int w, int h);

        /**
         * 根据纹理坐标获取YUV值，此时还不能显示
         * @param u  纹理坐标：相对位置（浮点数，0到1）
         * @param v 同 u
         * @param 像素索引：访问具体像素的位置（整数，0到width-1）
         */
        void sampleYUV(float u, float v, unsigned char &y, unsigned char &u_val, unsigned char &v_val);

        int getWidth() const { return width; }

        int getHeight() const { return height; }

    private:
        /// 为什么用std::vector<unsigned char>而不是float？
        /// 因为unsigned char兼顾了传输性能和图像质量，一般来讲8bit对于视觉上能接受的图片精度已然足够，
        /// 除非进行复杂的数字信号处理（颜色空间变换、hdr、滤镜渲染等）才需要将8bit数据转成flaot类型32bit。
        // I420格式，Y、U、V三个独立平面
        // Y平面：每个像素对应 1 个unsigned char 即 8 bit（1 byte）
        // U/V平面：每 4 个像素共享 1 个unsigned 即 8 bit（1 byte）
        std::vector<unsigned char> y_plane;
        std::vector<unsigned char> u_plane;
        std::vector<unsigned char> v_plane;
        int width, height; // 宽高
    };

} // namespace SoftRenderer

#endif /* YUVTexture_hpp */
