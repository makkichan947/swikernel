<h1 align="center">SwiKernel - Linux Kernel Switcher</h1>

<p align="center">
  <strong>简单、安全、强大的 Linux 内核管理工具</strong>
</p>

<p align="center">
  <a href="https://github.com//swikernel/releases">
    <img src="https://img.shields.io/github/v/release/makkichan947/swikernel?color=blue&logo=github" alt="GitHub Release">
  </a>
  <a href="https://github.com//swikernel/actions">
    <img src="https://img.shields.io/github/actions/workflow/status/makkichan947/swikernel/ci.yml?branch=main&logo=github" alt="Build Status">
  </a>
  <a href="LICENSE">
    <img src="https://img.shields.io/github/license/makkichan947/swikernel?color=green" alt="License">
  </a>
</p>

<p align="center">
  <a href="#特性">特性</a> •
  <a href="#快速开始">快速开始</a> •
  <a href="#安装">安装</a> •
  <a href="#使用指南">使用指南</a> •
  <a href="#文档">文档</a> •
  <a href="#贡献">贡献</a> •
  <a href="#许可证">许可证</a>
</p>

## 🌟 特性

### 🚀 核心功能
- **多内核管理**: 轻松安装、切换、删除多个 Linux 内核
- **一键编译**: 自动从源码编译和安装内核
- **智能依赖**: 自动检测和安装编译依赖
- **安全回滚**: 完整的回滚机制，确保系统安全
- **预设配置**: 针对不同用途的优化配置预设

### 🛡️ 安全可靠
- **签名验证**: 自动验证内核签名和完整性
- **备份保护**: 安装前自动备份系统配置
- **沙盒模式**: 安全的编译和测试环境
- **权限控制**: 细粒度的权限管理和审计

### 🎯 用户体验
- **直观 TUI**: 基于对话框的文本用户界面
- **命令行接口**: 完整的命令行工具
- **自动补全**: 智能路径和内核名称补全
- **进度显示**: 实时编译和安装进度
- **多语言支持**: 国际化和本地化

### 🔧 高级功能
- **交叉编译**: 支持多架构内核编译
- **自定义配置**: 灵活的内核配置管理
- **模块管理**: 内核模块的智能管理
- **性能优化**: 针对硬件的优化配置
- **插件系统**: 可扩展的插件架构

## 🚀 快速开始

### 系统要求
- **操作系统**: Linux (内核 4.4+)
- **架构**: x86_64, arm64, arm, ppc64le, riscv64
- **内存**: 2GB+ (编译推荐 4GB+)
- **存储**: 10GB+ 可用空间
- **权限**: Root 访问权限

### 5分钟安装体验

```bash
# 1. 下载安装脚本
curl -fsSL https://raw.githubusercontent.com/makkichan947/swikernel/main/scripts/install.sh -o install.sh

# 2. 运行安装
chmod +x install.sh
sudo ./install.sh

# 3. 启动 TUI 界面
sudo swikernel
```

### 基本使用示例

```bash
# 列出可用内核
swikernel --list-kernels

# 安装新内核
sudo swikernel --install linux-6.1

# 从源码安装
sudo swikernel --install-from-source /path/to/kernel/source --name my-custom-kernel

# 切换活动内核
sudo swikernel --switch-to linux-6.1

# 移除旧内核
sudo swikernel --remove linux-5.15
```

## 📦 安装

### 方法一: 一键安装 (推荐)
```bash
curl -fsSL https://raw.githubusercontent.com/makkichan947/swikernel/main/scripts/install.sh | bash
```

### 方法二: 从源码编译
```bash
git clone https://github.com/makkichan947/swikernel.git
cd swikernel
make deps
make release
sudo make install
```

## 📖 使用指南

### TUI 界面使用

启动文本用户界面:
```bash
sudo swikernel
```

主菜单选项:
1. **内核管理** - 查看、安装、删除内核
2. **源码编译** - 从源码编译自定义内核
3. **系统设置** - 配置 SwiKernel 行为
4. **工具集** - 高级工具和实用程序

### 命令行参考

#### 内核管理
```bash
# 列出所有可用内核
swikernel --list-kernels

# 安装指定版本内核
sudo swikernel --install linux-6.1.28

# 从源码安装
sudo swikernel --install-from-source /usr/src/linux-6.1 --name custom-6.1

# 切换活动内核
sudo swikernel --switch-to linux-6.1.28

# 移除旧内核
sudo swikernel --remove linux-5.15.90

# 设置默认内核
sudo swikernel --set-default linux-6.1.28
```

#### 系统管理
```bash
# 检查系统依赖
swikernel --check-dependencies

# 更新引导配置
sudo swikernel --update-bootloader

# 清理缓存和临时文件
swikernel --clean-cache

# 验证系统状态
swikernel --diagnose
```

#### 高级功能
```bash
# 交叉编译 (ARM64)
sudo swikernel --cross-compile arm64 --install-from-source /path/to/source

# 使用配置预设
sudo swikernel --install linux-6.1 --preset server

# 自定义编译选项
sudo swikernel --install-from-source /path/to/source --make-args="-j8"

# 生成调试报告
swikernel --generate-report
```

### 配置管理

SwiKernel 的配置文件位于 `/etc/swikernel/swikernel.conf`:

```ini
[logging]
level = INFO
file = /var/log/swikernel.log

[kernel]
default_source_dir = /usr/src
backup_enabled = true

[installation]
auto_reboot = false
timeout = 3600
```

### 预设配置

SwiKernel 提供多种内核配置预设:

- **minimal**: 最小化配置，适合嵌入式系统
- **desktop**: 桌面优化，支持多媒体和图形
- **server**: 服务器优化，强调稳定性和性能
- **performance**: 性能最大化，适合工作站
- **security**: 安全加固，增强系统安全性

使用预设:
```bash
sudo swikernel --install linux-6.1 --preset server
```

## 📚 文档

### 详细文档
- **[安装指南](docs/INSTALL.md)** - 完整的安装说明和故障排除
- **[故障排除](docs/TROUBLESHOOTING.md)** - 常见问题和解决方案

### 获取帮助
- **GitHub Issues**: [报告问题](https://github.com/makkichan947/swikernel/issues)
- **邮件**: makkichan947@hotmail.com / nekosparry0727@outlook.com
- **Stack Overflow**: 使用标签 `[swikernel]`

## 🤝 贡献

我们欢迎各种形式的贡献！

### 报告问题
如果你发现 bug 或有功能建议，请 [创建 Issue](https://github.com/makkichan947/swikernel/issues/new/choose)。

### 代码贡献
1. Fork 本项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

### 开发环境设置
```bash
# 克隆仓库
git clone https://github.com/makkichan947/swikernel.git
cd swikernel

# 安装开发依赖
make deps-dev

# 构建调试版本
make debug

# 运行测试
make test

# 代码格式检查
make format-check
```


## 📄 许可证

本项目采用 **NC-OSL** 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。


---

<div align="center">

**如果这个项目对你有帮助，请给它一个 ⭐️ 星标！**

[![Star History Chart](https://api.star-history.com/svg?repos=makkichan947/swikernel&type=Date)](https://star-history.com/#makkichan947swikernel&Date)

</div>
