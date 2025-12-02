#!/bin/bash
echo "🧹 清理项目..."

# 删除CMake生成的文件
rm -rf CMakeCache.txt CMakeFiles Makefile cmake_install.cmake 2>/dev/null

# 删除.cmake文件（如果有）
find . -maxdepth 1 -name "*.cmake" -delete 2>/dev/null

# 删除Xcode项目
rm -rf SoftRenderer.xcodeproj 2>/dev/null

# 删除构建目录
rm -rf build xcode-build .xcode-build 2>/dev/null

echo "✅ 清理完成！"