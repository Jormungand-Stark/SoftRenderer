#!/usr/bin/env python3
"""
SoftRenderer - YUV测试文件生成脚本
生成I420(YUV420p)格式的测试图案文件
"""

import numpy as np
import os
import sys
from pathlib import Path

def change_to_script_dir():
    """切换到脚本所在的目录"""
    script_dir = os.path.dirname(os.path.realpath(__file__))
    os.chdir(script_dir)
    print(f"工作目录已切换到: {os.getcwd()}")

def generate_test_pattern(width, height):
    """
    生成测试图案
    返回: (y_plane, u_plane, v_plane) 三个numpy数组
    """
    # 创建平面
    y_plane = np.zeros((height, width), dtype=np.uint8)
    u_plane = np.zeros((height // 2, width // 2), dtype=np.uint8)
    v_plane = np.zeros((height // 2, width // 2), dtype=np.uint8)
    
    # 生成水平渐变 + 垂直渐变 + 对角线图案的Y分量
    for y in range(height):
        for x in range(width):
            # 水平渐变 (0-255)
            horizontal = int((x / max(width-1, 1)) * 255)
            # 垂直渐变 (0-255)
            vertical = int((y / max(height-1, 1)) * 255)
            # 对角线图案
            diagonal = int(((x + y) / (width + height - 2)) * 255) if (width + height) > 2 else 0
            # 综合：棋盘格效果
            if (x // 40 + y // 40) % 2 == 0:
                pattern = 200  # 亮块
            else:
                pattern = 50   # 暗块
            
            # 混合效果
            y_value = int(horizontal * 0.3 + vertical * 0.3 + diagonal * 0.2 + pattern * 0.2)
            y_plane[y, x] = np.clip(y_value, 0, 255)
    
    # 生成UV平面 - 创建彩色区域
    # 左上角: 偏红色 (V > 128)
    # 右上角: 偏绿色 (U~128, V~128)
    # 左下角: 偏蓝色 (U > 128)
    # 右下角: 偏黄色 (U>128, V>128)
    
    uv_height, uv_width = u_plane.shape
    
    for v in range(uv_height):
        for u in range(uv_width):
            # 水平渐变
            u_gradient = int((u / max(uv_width-1, 1)) * 255)
            v_gradient = int((v / max(uv_height-1, 1)) * 255)
            
            # 四个象限的不同颜色
            if u < uv_width // 2 and v < uv_height // 2:  # 左上
                u_val = 128
                v_val = 200  # 偏红
            elif u >= uv_width // 2 and v < uv_height // 2:  # 右上
                u_val = 128
                v_val = 128  # 灰色
            elif u < uv_width // 2 and v >= uv_height // 2:  # 左下
                u_val = 200  # 偏蓝
                v_val = 128
            else:  # 右下
                u_val = 200  # 偏黄
                v_val = 200
            
            # 添加渐变过渡
            u_plane[v, u] = np.clip(int(u_val * 0.7 + u_gradient * 0.3), 0, 255)
            v_plane[v, u] = np.clip(int(v_val * 0.7 + v_gradient * 0.3), 0, 255)
    
    return y_plane, u_plane, v_plane

def save_yuv_file(y_plane, u_plane, v_plane, output_path):
    """保存为I420格式的YUV文件"""
    try:
        with open(output_path, 'wb') as f:
            f.write(y_plane.tobytes())
            f.write(u_plane.tobytes())
            f.write(v_plane.tobytes())
        return True
    except Exception as e:
        print(f"保存文件时出错: {e}")
        return False

def print_file_info(width, height, output_path):
    """打印生成的文件信息"""
    file_size = width * height * 3 // 2  # YUV420大小
    y_size = width * height
    uv_size = (width // 2) * (height // 2)
    
    print("\n" + "="*50)
    print("YUV测试文件生成成功!")
    print("="*50)
    print(f"图像尺寸:  {width} x {height}")
    print(f"Y平面大小: {y_size:,} 字节")
    print(f"U平面大小: {uv_size:,} 字节")
    print(f"V平面大小: {uv_size:,} 字节")
    print(f"文件大小:  {file_size:,} 字节 ({file_size/1024:.1f} KB)")
    print(f"保存路径:  {output_path}")
    print("="*50)
    print("文件格式: I420 (YUV420 planar)")
    print(f"数据布局: [Y:{y_size}字节][U:{uv_size}字节][V:{uv_size}字节]")
    print("="*50)

def main():
    """主函数"""
    print("SoftRenderer - YUV测试文件生成器")
    print("-" * 40)
    
    # 切换到脚本所在目录，便于使用相对路径
    change_to_script_dir()
    
    # 检查参数
    if len(sys.argv) != 4:
        print("用法: python generate_test_yuv.py <宽度> <高度> <输出文件>")
        print("示例: python generate_test_yuv.py 640 480 ../yuv/test_640x480.yuv")
        print("\n参数说明:")
        print("  宽度: 图像的宽度 (像素)")
        print("  高度: 图像的高度 (像素)")
        print("  输出文件: YUV文件的保存路径")
        return 1
    
    try:
        # 解析参数
        width = int(sys.argv[1])
        height = int(sys.argv[2])
        output_path = sys.argv[3]
        
        # 验证参数
        if width <= 0 or height <= 0:
            print("错误: 宽度和高度必须是正整数")
            return 1
        
        if width % 2 != 0 or height % 2 != 0:
            print("警告: YUV420格式要求宽高为偶数")
            print("已自动调整尺寸:")
            width = width if width % 2 == 0 else width + 1
            height = height if height % 2 == 0 else height + 1
            print(f"  宽度: {width}, 高度: {height}")
        
        # 确保输出目录存在
        output_dir = os.path.dirname(output_path)
        if output_dir and not os.path.exists(output_dir):
            os.makedirs(output_dir, exist_ok=True) # 自动创建目录
            print(f"创建目录: {output_dir}")
        
        # 生成测试图案
        print(f"生成 {width}x{height} 测试图案...")
        y_plane, u_plane, v_plane = generate_test_pattern(width, height)
        
        # 保存文件
        print(f"保存到: {output_path}")
        if save_yuv_file(y_plane, u_plane, v_plane, output_path):
            # 打印文件信息
            print_file_info(width, height, output_path)
            
            # 生成使用示例
            print("\n在SoftRenderer项目中的使用示例:")
            print("-" * 40)
            print(f"C++代码:")
            print(f'  YUVTexture texture("{os.path.basename(output_path)}", {width}, {height});')
            print(f"\n编译运行:")
            print(f'  ./softrender "{output_path}" {width} {height}')
            print("-" * 40)
            
            return 0
        else:
            return 1
            
    except ValueError:
        print("错误: 宽度和高度必须是整数")
        return 1
    except KeyboardInterrupt:
        print("\n\n用户中断操作")
        return 1
    except Exception as e:
        print(f"未预期的错误: {e}")
        return 1

if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)