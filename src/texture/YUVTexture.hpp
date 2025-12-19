//
//  YUVTexture.hpp
//  softrender
//
//  Created by Jormungand on 2025/11/20.
//

#ifndef YUVTexture_hpp
#define YUVTexture_hpp

#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

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
// 数据输入层，存储和管理原始图像数据，为渲染管线提供纹理采样服务，工作在YUV色彩空间，需要转换才能显示。
namespace SoftRenderer {
     /**
      * YUVTexture 的每个点是 纹理像素（Texel，Texture Element）。它是存储在纹理数据结构中的最小数据单元（本项目中为 YUV 颜色分量），
      * 用于纹理映射和采样。与屏幕像素（Pixel）不同，纹理像素不直接对应于屏幕上的显示位置，而是通过纹理坐标映射到几何体表面。
      */
     enum class TextureFilter {
          NEAREST,  // 最近点采样
          BILINEAR  // 双线性插值
     };

     class YUVTexture {
     public:
          // 从文件加载数据
          YUVTexture(const std::string &filename, int w, int h);

          void setFilterMode(TextureFilter mode) { filter_mode_ = mode; }

          /**
           * 根据纹理坐标获取YUV值，此时还不能显示
           * @param u 归一化水平纹理坐标u [0,1]，0=左边界，1=右边界
           * @param v 归一化垂直纹理坐标v [0,1]，0=上边界，1=下边界
           * @param y_val 输出：亮度分量Y
           * @param u_val 输出：色度分量U（为避免命名冲突）
           * @param v_val 输出：色度分量V（为避免命名冲突）
           */
          void sampleYUV(float u, float v,
                         unsigned char &y_val, unsigned char &u_val, unsigned char &v_val) const;

          int getWidth() const { return width_; }

          int getHeight() const { return height_; }

     private:
          void sampleNearest(float u, float v,
                             unsigned char &y_val, unsigned char &u_val, unsigned char &v_val) const;
          float samplePlaneBilinear(const std::vector<unsigned char> &plane,
                                    int planeWidth, int planeHeight,
                                    float u, float v) const;
          void sampleBilinear(float u, float v,
                              unsigned char &y_val, unsigned char &u_val, unsigned char &v_val) const;
          // 纹理过滤模式，使用成员变量一次设定每次采样受益。也更符合现代图形API的“状态机”模型设计思路。
          TextureFilter filter_mode_ = TextureFilter::NEAREST;

          /// 为什么用std::vector<unsigned char>而不是float？
          /// 因为unsigned char兼顾了传输性能和图像质量，一般来讲8bit对于视觉上能接受的图片精度已然足够，
          /// 除非进行复杂的数字信号处理（颜色空间变换、hdr、滤镜渲染等）才需要将8bit数据转成flaot类型32bit。
          // I420格式，Y、U、V三个独立平面
          // Y平面：每个像素对应 1 个unsigned char 即 8 bit（1 byte）
          // U/V平面：每 4 个像素共享 1 个unsigned 即 8 bit（1 byte）
          std::vector<unsigned char> y_plane_;
          std::vector<unsigned char> u_plane_;
          std::vector<unsigned char> v_plane_;

          int width_, height_; // 宽高
     };

} // namespace SoftRenderer

#endif /* YUVTexture_hpp */
