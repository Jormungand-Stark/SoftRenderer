#!/bin/bash
# setup_xcode.sh - 创建标准Xcode构建环境
# 用法: ./scripts/setup_xcode.sh [Debug|Release]

set -e  # 遇到错误立即退出

BUILD_TYPE="${1:-Debug}"
BUILD_DIR="build/xcode-${BUILD_TYPE}"

echo "🔄 为SoftRenderer设置Xcode构建环境..."
echo "📁 构建目录: ${BUILD_DIR}"
echo "⚙️  构建类型: ${BUILD_TYPE}"
echo ""

# 1. 创建构建目录
echo "📂 创建构建目录..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# 2. 运行CMake生成Xcode项目
echo "🔧 运行CMake生成Xcode项目..."
echo "   命令: cmake -G Xcode -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ../.."
echo ""

cmake -G "Xcode" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_XCODE_ATTRIBUTE_ENABLE_DTRACE=NO \
    ../..

# 3. 验证生成结果
echo ""
echo "🔍 验证生成结果..."

# 查找生成的Xcode项目 - 优先找我们的主项目
MAIN_PROJECT=""
if [[ -d "SoftRenderer.xcodeproj" ]]; then
    MAIN_PROJECT="SoftRenderer.xcodeproj"
else
    # 如果不在当前目录，搜索整个目录
    PROJECT_FILES=$(find . -name "*.xcodeproj" -type d 2>/dev/null || true)
    if [[ -z "${PROJECT_FILES}" ]]; then
        echo "❌ 错误: 未生成Xcode项目文件"
        exit 1
    fi
    
    # 过滤掉CMake的测试项目
    MAIN_PROJECT=$(echo "${PROJECT_FILES}" | grep -v "CMakeFiles" | grep -v "CompilerId" | head -1)
    
    # 如果过滤后没有，用第一个（作为后备）
    if [[ -z "${MAIN_PROJECT}" ]]; then
        MAIN_PROJECT=$(echo "${PROJECT_FILES}" | head -1)
        echo "⚠️  警告: 只找到CMake测试项目，使用: ${MAIN_PROJECT}"
    fi
fi

if [[ -z "${MAIN_PROJECT}" ]]; then
    echo "❌ 错误: 未找到主Xcode项目文件"
    exit 1
fi

# 显示生成的文件
PROJECT_FILES=$(find . -name "*.xcodeproj" -type d 2>/dev/null || true)
PROJECT_COUNT=$(echo "${PROJECT_FILES}" | wc -l | tr -d ' ')
echo "✅ 成功生成 ${PROJECT_COUNT} 个Xcode项目文件:"
echo "${PROJECT_FILES}" | sed 's/^/   /'

echo ""
echo "🎯 主项目文件: $(pwd)/${MAIN_PROJECT#./}"
echo ""
echo "📋 后续操作建议:"
echo "   打开项目: open $(pwd)/${MAIN_PROJECT#./}"
echo "   或使用: ./scripts/open_xcode.sh ${BUILD_TYPE}"
echo ""
echo "💡 提示:"
echo "   清理此配置: rm -rf $(pwd)"
echo "   创建其他配置: ./scripts/setup_xcode.sh Release"