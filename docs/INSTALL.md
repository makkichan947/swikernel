# SwiKernel 安装指南

## 目录
- [系统要求](#系统要求)
- [快速安装](#快速安装)
- [从源码编译安装](#从源码编译安装)
- [验证安装](#验证安装)
- [故障排除](#故障排除)

## 系统要求

### 最低要求
- **操作系统**: Linux 内核 4.4+ (推荐 5.10+)
- **架构**: x86_64, arm64, arm, ppc64le, riscv64
- **内存**: 2GB RAM (编译时需要 4GB+)
- **存储**: 10GB 可用空间
- **权限**: Root 权限 (用于内核安装)

### 软件依赖
- **编译器**: GCC 7.5+ 或 Clang 10+
- **构建工具**: Make, CMake 3.12+
- **汇编器**: NASM 2.14+
- **库文件**: 
  - libdialog (用于 TUI 界面)
  - libncurses (终端控制)
  - libelf (ELF 文件处理)
  - libssl (加密和签名验证)

### 内核编译依赖
```bash
# Debian/Ubuntu
build-essential gcc g++ make cmake nasm \
libc6-dev linux-headers-$(uname -r) pkg-config \
bc bison flex libelf-dev libssl-dev rsync cpio xz-utils \
dialog ncurses-dev git wget curl file patchelf

# RHEL/CentOS/Fedora
gcc gcc-c++ make cmake nasm glibc-devel \
kernel-devel pkgconfig bc bison flex \
elfutils-libelf-devel openssl-devel rsync cpio xz \
dialog ncurses-devel git wget curl file patchelf

# Arch Linux
base-devel gcc make cmake nasm linux-headers \
pkg-config bc bison flex elfutils openssl \
rsync cpio xz dialog ncurses git wget curl file patchelf
```

## 快速安装

### 一键安装脚本
```bash
# 下载并运行安装脚本
curl -fsSL https://raw.githubusercontent.com/makkichan947/swikernel/main/scripts/install.sh | bash

# 或者使用 wget
wget -qO- https://raw.githubusercontent.com/makkichan947/swikernel/main/scripts/install.sh | bash
```

### 分步安装
```bash
# 1. 下载最新发布版本
wget https://github.com/makkichan947/swikernel/releases/latest/download/swikernel-linux-x86_64.tar.gz

# 2. 解压文件
tar -xzf swikernel-linux-x86_64.tar.gz
cd swikernel

# 3. 运行安装程序
sudo ./install.sh

# 4. 验证安装
swikernel --version
```

## 从源码编译安装

### 获取源码
```bash
# 从 GitHub 克隆
git clone https://github.com/makkichan947/swikernel.git
cd swikernel

# 或者下载源码包
wget https://github.com/makkichan947/swikernel/archive/refs/heads/main.zip
unzip main.zip
cd swikernel-main
```

### 编译安装
```bash
# 方法一: 使用 Makefile (推荐)
make deps          # 安装依赖
make release       # 发布版本编译
sudo make install  # 安装到系统

# 方法二: 使用 CMake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install

# 方法三: 调试版本
make debug
sudo make install
```

### 编译选项
```bash
# 自定义安装前缀
make install PREFIX=/usr/local

# 静态链接编译
make static

# 最小化编译
make minimal

# 交叉编译
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
```

## 验证安装

### 基本验证
```bash
# 检查版本
swikernel --version

# 检查帮助
swikernel --help

# 列出可用命令
swikernel --list-commands
```

### 功能测试
```bash
# 测试内核列表功能
swikernel --list-kernels

# 测试依赖检查
swikernel --check-dependencies

# 测试配置验证
swikernel --validate-config
```

### 系统集成检查
```bash
# 检查服务状态
systemctl status swikernel

# 检查日志
journalctl -u swikernel

# 验证文件权限
ls -la /usr/local/bin/swikernel
```

## 故障排除

### 常见问题

#### 1. 权限错误
```bash
# 错误: Permission denied
sudo swikernel [command]

# 或者将用户添加到 sudoers
sudo usermod -aG sudo $USER
```

#### 2. 依赖缺失
```bash
# 自动安装依赖
sudo swikernel --install-dependencies

# 手动安装
sudo ./scripts/setup_deps.sh
```

#### 3. 编译错误
```bash
# 清理构建缓存
make clean

# 更新工具链
sudo apt update && sudo apt upgrade

# 检查编译器版本
gcc --version
make --version
```

#### 4. 内核安装失败
```bash
# 检查引导加载器
sudo update-grub

# 检查磁盘空间
df -h /boot

# 检查当前内核
uname -r
```

#### 5. TUI 界面问题
```bash
# 检查终端兼容性
echo $TERM

# 使用命令行模式
swikernel --no-tui

# 设置终端类型
export TERM=xterm-256color
```

### 获取帮助

#### 社区支持
- **GitHub Issues**: [报告问题](https://github.com/makkichan947/swikernel/issues)
- **邮件**: makkichan947@hotmail.com / nekosparry0727@outlook.com

#### 调试模式
```bash
# 启用详细输出
swikernel --verbose

# 启用调试模式
swikernel --debug

# 生成调试报告
swikernel --generate-debug-report
```

## 更新和卸载

### 更新 SwiKernel
```bash
# 包管理器更新
sudo apt update && sudo apt upgrade swikernel

# 源码更新
git pull
make clean
make release
sudo make install
```

### 卸载 SwiKernel
```bash
# 包管理器卸载
sudo apt remove swikernel

# 源码卸载
sudo make uninstall

# 彻底清理
sudo rm -rf /etc/swikernel
sudo rm -rf /var/lib/swikernel
sudo rm -rf /var/log/swikernel
```

---

**注意**: 安装过程中如遇问题，请查看详细日志：`/var/log/swikernel/install.log`