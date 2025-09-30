#!/bin/bash

# SwiKernel Dependency Installer
# This script installs all required dependencies for SwiKernel

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检测包管理器
detect_package_manager() {
    if command -v apt-get >/dev/null 2>&1; then
        echo "apt"
    elif command -v yum >/dev/null 2>&1; then
        echo "yum"
    elif command -v dnf >/dev/null 2>&1; then
        echo "dnf"
    elif command -v pacman >/dev/null 2>&1; then
        echo "pacman"
    elif command -v zypper >/dev/null 2>&1; then
        echo "zypper"
    else
        log_error "Unable to detect package manager"
        exit 1
    fi
}

# 检查root权限
check_root() {
    if [[ $EUID -eq 0 ]]; then
        log_info "Running with root privileges"
    else
        log_error "This script requires root privileges. Please run with sudo."
        exit 1
    fi
}

# 安装依赖 (APT)
install_apt_deps() {
    log_info "Installing dependencies using APT..."
    
    apt-get update
    
    # 必需依赖
    local essential_deps=(
        "build-essential"
        "gcc"
        "g++"
        "make"
        "cmake"
        "nasm"
        "libc6-dev"
        "linux-headers-$(uname -r)"
        "pkg-config"
    )
    
    # 内核编译依赖
    local kernel_deps=(
        "bc"
        "bison"
        "flex"
        "libelf-dev"
        "libssl-dev"
        "rsync"
        "cpio"
        "xz-utils"
    )
    
    # 工具依赖
    local tool_deps=(
        "dialog"
        "ncurses-dev"
        "git"
        "wget"
        "curl"
        "file"
        "patchelf"
    )
    
    # 安装所有依赖
    apt-get install -y \
        "${essential_deps[@]}" \
        "${kernel_deps[@]}" \
        "${tool_deps[@]}"
}

# 安装依赖 (YUM/DNF)
install_yum_deps() {
    local pm=$1
    log_info "Installing dependencies using $pm..."
    
    $pm update -y
    
    # 必需依赖
    local essential_deps=(
        "gcc"
        "gcc-c++"
        "make"
        "cmake"
        "nasm"
        "glibc-devel"
        "kernel-devel"
        "pkgconfig"
    )
    
    # 内核编译依赖
    local kernel_deps=(
        "bc"
        "bison"
        "flex"
        "elfutils-libelf-devel"
        "openssl-devel"
        "rsync"
        "cpio"
        "xz"
    )
    
    # 工具依赖
    local tool_deps=(
        "dialog"
        "ncurses-devel"
        "git"
        "wget"
        "curl"
        "file"
        "patchelf"
    )
    
    # 安装所有依赖
    $pm install -y \
        "${essential_deps[@]}" \
        "${kernel_deps[@]}" \
        "${tool_deps[@]}"
}

# 安装依赖 (PACMAN)
install_pacman_deps() {
    log_info "Installing dependencies using PACMAN..."
    
    pacman -Syu --noconfirm
    
    # 必需依赖
    local essential_deps=(
        "base-devel"
        "gcc"
        "make"
        "cmake"
        "nasm"
        "linux-headers"
        "pkg-config"
    )
    
    # 内核编译依赖
    local kernel_deps=(
        "bc"
        "bison"
        "flex"
        "elfutils"
        "openssl"
        "rsync"
        "cpio"
        "xz"
    )
    
    # 工具依赖
    local tool_deps=(
        "dialog"
        "ncurses"
        "git"
        "wget"
        "curl"
        "file"
        "patchelf"
    )
    
    # 安装所有依赖
    pacman -S --noconfirm \
        "${essential_deps[@]}" \
        "${kernel_deps[@]}" \
        "${tool_deps[@]}"
}

# 验证安装
verify_installation() {
    log_info "Verifying installation..."
    
    local tools=("gcc" "make" "cmake" "nasm" "dialog" "git")
    local missing_tools=()
    
    for tool in "${tools[@]}"; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            missing_tools+=("$tool")
        fi
    done
    
    if [[ ${#missing_tools[@]} -eq 0 ]]; then
        log_success "All essential tools are installed and available"
    else
        log_error "Missing tools: ${missing_tools[*]}"
        return 1
    fi
}

# 设置环境
setup_environment() {
    log_info "Setting up environment..."
    
    # 创建配置目录
    mkdir -p /etc/swikernel
    mkdir -p /var/log/swikernel
    mkdir -p /var/lib/swikernel/backups
    
    # 设置权限
    chmod 755 /etc/swikernel
    chmod 755 /var/log/swikernel
    chmod 700 /var/lib/swikernel
    
    log_success "Environment setup completed"
}

# 主函数
main() {
    log_info "Starting SwiKernel dependency installation..."
    
    # 检查root权限
    check_root
    
    # 检测包管理器
    local pm=$(detect_package_manager)
    log_info "Detected package manager: $pm"
    
    # 安装依赖
    case $pm in
        "apt")
            install_apt_deps
            ;;
        "yum"|"dnf")
            install_yum_deps "$pm"
            ;;
        "pacman")
            install_pacman_deps
            ;;
        *)
            log_error "Unsupported package manager: $pm"
            exit 1
            ;;
    esac
    
    # 验证安装
    if verify_installation; then
        log_success "Dependency installation completed successfully"
    else
        log_error "Dependency installation completed with errors"
        exit 1
    fi
    
    # 设置环境
    setup_environment
    
    log_success "SwiKernel dependency installation completed!"
    log_info "You can now build and use SwiKernel"
}

# 脚本入口
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi