#!/bin/bash
# open_xcode.sh - 智能打开或创建Xcode项目
# 用法: ./scripts/open_xcode.sh [Debug|Release]

set -e

CONFIG="${1:-Debug}"
BUILD_DIR="build/xcode-${CONFIG}"

echo "🔍 检查Xcode项目 (${CONFIG}配置)..."

# 更可靠的查找函数
find_project_file() {
    local dir="$1"
    
    # 检查目录是否存在
    if [[ ! -d "${dir}" ]]; then
        return 1
    fi
    
    # 方法1：直接找项目根目录下的.xcodeproj（最常见）
    if [[ -d "${dir}/SoftRenderer.xcodeproj" ]]; then
        echo "${dir}/SoftRenderer.xcodeproj"
        return 0
    fi
    
    # 方法2：在整个构建目录中搜索，排除CMake内部项目
    local found=""
    if found=$(find "${dir}" -name "*.xcodeproj" -type d 2>/dev/null | grep -v "/CMakeFiles/" | grep -v "CompilerId" | head -1); then
        if [[ -n "${found}" ]]; then
            echo "${found}"
            return 0
        fi
    fi
    
    # 方法3：如果还是找不到，返回第一个找到的（作为后备）
    if found=$(find "${dir}" -name "*.xcodeproj" -type d 2>/dev/null | head -1); then
        if [[ -n "${found}" ]]; then
            echo "⚠️  警告: 只找到CMake测试项目: ${found}" >&2
            echo "${found}"
            return 0
        fi
    fi
    
    return 1
}

# 查找项目文件（允许失败）
PROJECT_FILE=$(find_project_file "${BUILD_DIR}" 2>/dev/null || true)

# 检查项目是否存在
if [[ -n "${PROJECT_FILE}" && -d "${PROJECT_FILE}" ]]; then
    echo "📂 找到现有项目: ${PROJECT_FILE}"
    echo "🚀 正在打开..."
    open "${PROJECT_FILE}"
    exit 0
fi

# 项目不存在，询问用户
echo "📭 未找到现有项目"
read -p "❓ 是否创建新的 ${CONFIG} 配置Xcode项目？ [Y/n]: " -n 1 -r
echo  # 换行

if [[ "${REPLY}" =~ ^[Nn]$ ]]; then
    echo "❌ 操作已取消"
    exit 0
fi

# 调用setup脚本创建项目
echo "🔧 正在创建项目..."
if ! ./scripts/setup_xcode.sh "${CONFIG}"; then
    echo "❌ 项目创建失败"
    exit 1
fi

# 再次尝试查找
PROJECT_FILE=$(find_project_file "${BUILD_DIR}")

if [[ -n "${PROJECT_FILE}" && -d "${PROJECT_FILE}" ]]; then
    echo "✅ 项目创建成功！"
    echo "🎯 项目位置: ${PROJECT_FILE}"
    echo "🚀 正在打开..."
    open "${PROJECT_FILE}"
else
    echo "❌ 错误: 项目文件未找到"
    echo "调试信息:"
    echo "  构建目录: $(pwd)/${BUILD_DIR}"
    if [[ -d "${BUILD_DIR}" ]]; then
        echo "  目录内容:"
        ls -la "${BUILD_DIR}"
    else
        echo "  目录不存在"
    fi
    exit 1
fi