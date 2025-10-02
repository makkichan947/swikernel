#!/bin/bash

# SwiKernel GitHub 推送脚本
# 使用方法: ./push_to_github.sh

echo "🚀 推送 SwiKernel 更新到 GitHub..."

# 检查是否有未推送的提交
if git log --oneline origin/main..HEAD | grep -q "."; then
    echo "📝 发现新的提交，准备推送..."

    # 推送主分支
    echo "🔄 推送中..."
    if git push origin main; then
        echo "✅ 推送成功！"
        echo ""
        echo "🎉 更新已发布到 GitHub！"
        echo "📊 本次更新统计:"
        git show --stat HEAD | tail -1
        echo ""
        echo "🌟 主要新功能:"
        echo "  • 国际化支持 (10种语言)"
        echo "  • 文件管理器"
        echo "  • 日志查看器"
        echo "  • 配置管理器"
        echo "  • 插件系统"
        echo "  • 系统监控仪表盘"
        echo "  • 主题管理器"
        echo "  • 布局管理器"
        echo "  • 增强的错误处理"
        echo ""
        echo "📚 文档: README.md (英文) + ZH.README.md (中文)"
    else
        echo "❌ 推送失败，请检查网络连接和凭据"
        exit 1
    fi
else
    echo "ℹ️  本地仓库已是最新状态，无需推送"
fi