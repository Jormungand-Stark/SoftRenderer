#!/bin/bash
echo "🔄 生成Xcode项目..."

# 清理
./scripts/clean.sh

# 生成
cmake -G Xcode .

# 清理临时文件
rm -rf CMakeCache.txt CMakeFiles *.cmake Makefile cmake_install.cmake 2>/dev/null

echo "✅ 项目已生成：SoftRenderer.xcodeproj"
echo "📱 用以下命令打开：open SoftRenderer.xcodeproj"