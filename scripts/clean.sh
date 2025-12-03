#!/bin/bash
# clean.sh - 安全清理构建产物
# 用法: ./scripts/clean.sh [all|build|cache]

MODE="${1:-interactive}"

echo "🧹 SoftRenderer项目清理工具"
echo "=============================="

case "${MODE}" in
    "all")
        echo "🗑️  执行完全清理..."
        # 清理构建目录
        if [[ -d "build" ]]; then
            echo "   删除构建目录: build/"
            rm -rf build
        fi
        
        # 清理CMake缓存文件
        echo "   清理CMake缓存文件..."
        rm -rf CMakeCache.txt CMakeFiles Makefile cmake_install.cmake 2>/dev/null || true
        
        # 清理可能的其他构建目录
        echo "   清理其他构建目录..."
        rm -rf xcode-build .xcode-build _build 2>/dev/null || true
        
        echo "✅ 完全清理完成"
        ;;
        
    "build")
        echo "🗑️  只清理构建目录..."
        if [[ -d "build" ]]; then
            echo "   删除: build/"
            rm -rf build
            echo "✅ 构建目录已清理"
        else
            echo "📭 build/目录不存在，无需清理"
        fi
        ;;
        
    "cache")
        echo "🗑️  只清理CMake缓存..."
        CACHE_FILES=("CMakeCache.txt" "CMakeFiles" "Makefile" "cmake_install.cmake")
        CLEANED=0
        
        for file in "${CACHE_FILES[@]}"; do
            if [[ -e "${file}" ]]; then
                echo "   删除: ${file}"
                rm -rf "${file}"
                CLEANED=1
            fi
        done
        
        if [[ ${CLEANED} -eq 1 ]]; then
            echo "✅ CMake缓存已清理"
        else
            echo "📭 未找到CMake缓存文件"
        fi
        ;;
        
    "interactive"|*)
        echo "请选择清理模式:"
        echo "  1) 只清理构建目录 (build/)"
        echo "  2) 只清理CMake缓存文件"
        echo "  3) 完全清理 (构建目录+缓存)"
        echo "  4) 取消"
        echo ""
        read -p "请输入选择 [1-4]: " -n 1 -r
        echo  # 换行
        
        case "${REPLY}" in
            1)
                ./scripts/clean.sh build
                ;;
            2)
                ./scripts/clean.sh cache
                ;;
            3)
                ./scripts/clean.sh all
                ;;
            4|*)
                echo "❌ 操作取消"
                exit 0
                ;;
        esac
        ;;
esac

echo ""
echo "💡 提示: 清理操作不会删除源代码或配置文件"