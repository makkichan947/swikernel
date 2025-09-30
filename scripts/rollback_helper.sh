#!/bin/bash

# SwiKernel Rollback Helper Script
# This script helps with kernel rollback operations

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

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

# 检查root权限
check_root() {
    if [[ $EUID -ne 0 ]]; then
        log_error "This script requires root privileges. Please run with sudo."
        exit 1
    fi
}

# 显示当前内核信息
show_current_kernel() {
    log_info "Current kernel information:"
    echo "Kernel release: $(uname -r)"
    echo "Kernel version: $(uname -v)"
    echo "Machine architecture: $(uname -m)"
    echo
}

# 列出可用的内核
list_available_kernels() {
    log_info "Available installed kernels:"
    
    if command -v dpkg >/dev/null 2>&1; then
        # Debian/Ubuntu
        dpkg -l | grep 'linux-image' | grep '^ii' | awk '{print $2 " " $3}' || true
    elif command -v rpm >/dev/null 2>&1; then
        # RHEL/CentOS/Fedora
        rpm -qa | grep '^kernel' | sort || true
    elif command -v pacman >/dev/null 2>&1; then
        # Arch Linux
        pacman -Q | grep '^linux' || true
    else
        # Generic method
        find /boot -name 'vmlinuz-*' -exec basename {} \; | sed 's/vmlinuz-//' | sort
    fi
    echo
}

# 检查引导加载器
detect_bootloader() {
    if [[ -d "/boot/grub" ]]; then
        echo "grub"
    elif [[ -d "/boot/efi" ]]; then
        echo "efi"
    elif command -v systemctl >/dev/null 2>&1 && systemctl is-active --quiet bootctl; then
        echo "systemd-boot"
    else
        echo "unknown"
    fi
}

# 备份当前配置
backup_configuration() {
    local backup_dir="/var/lib/swikernel/backups/$(date +%Y%m%d_%H%M%S)"
    
    log_info "Creating configuration backup in: $backup_dir"
    
    mkdir -p "$backup_dir"
    
    # 备份引导配置
    if [[ -f "/boot/grub/grub.cfg" ]]; then
        cp "/boot/grub/grub.cfg" "$backup_dir/grub.cfg.bak"
    fi
    
    if [[ -f "/etc/default/grub" ]]; then
        cp "/etc/default/grub" "$backup_dir/grub.default.bak"
    fi
    
    # 备份内核配置
    if [[ -f "/boot/config-$(uname -r)" ]]; then
        cp "/boot/config-$(uname -r)" "$backup_dir/kernel-config.bak"
    fi
    
    # 备份模块配置
    if [[ -d "/etc/modprobe.d" ]]; then
        cp -r "/etc/modprobe.d" "$backup_dir/modprobe.d.bak"
    fi
    
    log_success "Configuration backup created: $backup_dir"
    echo "$backup_dir"
}

# 移除内核包
remove_kernel_package() {
    local kernel_name=$1
    
    log_info "Removing kernel: $kernel_name"
    
    if command -v dpkg >/dev/null 2>&1; then
        # Debian/Ubuntu
        local pkg_name="linux-image-${kernel_name}"
        if dpkg -l | grep -q "$pkg_name"; then
            dpkg --purge "$pkg_name"
            log_success "Removed kernel package: $pkg_name"
        else
            log_warning "Kernel package not found: $pkg_name"
        fi
    elif command -v rpm >/dev/null 2>&1; then
        # RHEL/CentOS/Fedora
        if rpm -q "kernel-$kernel_name" >/dev/null 2>&1; then
            rpm -e "kernel-$kernel_name"
            log_success "Removed kernel package: kernel-$kernel_name"
        else
            log_warning "Kernel package not found: kernel-$kernel_name"
        fi
    else
        log_warning "Package manager not found, performing manual cleanup"
        remove_kernel_manually "$kernel_name"
    fi
}

# 手动移除内核文件
remove_kernel_manually() {
    local kernel_name=$1
    
    log_info "Performing manual cleanup for kernel: $kernel_name"
    
    local files_to_remove=(
        "/boot/vmlinuz-${kernel_name}"
        "/boot/initrd.img-${kernel_name}"
        "/boot/System.map-${kernel_name}"
        "/boot/config-${kernel_name}"
    )
    
    for file in "${files_to_remove[@]}"; do
        if [[ -f "$file" ]]; then
            rm -f "$file"
            log_info "Removed: $file"
        fi
    done
    
    # 移除模块目录
    local modules_dir="/lib/modules/${kernel_name}"
    if [[ -d "$modules_dir" ]]; then
        rm -rf "$modules_dir"
        log_info "Removed modules directory: $modules_dir"
    fi
    
    log_success "Manual cleanup completed for kernel: $kernel_name"
}

# 更新引导加载器
update_bootloader() {
    local bootloader=$(detect_bootloader)
    
    log_info "Updating bootloader: $bootloader"
    
    case "$bootloader" in
        "grub")
            if command -v update-grub >/dev/null 2>&1; then
                update-grub
            elif command -v grub-mkconfig >/dev/null 2>&1; then
                grub-mkconfig -o /boot/grub/grub.cfg
            else
                log_warning "GRUB update commands not found"
            fi
            ;;
        "efi")
            log_info "EFI bootloader update may require manual intervention"
            ;;
        "systemd-boot")
            if command -v bootctl >/dev/null 2>&1; then
                bootctl update
            else
                log_warning "bootctl not found"
            fi
            ;;
        *)
            log_warning "Unknown bootloader, please update manually"
            ;;
    esac
    
    log_success "Bootloader update completed"
}

# 回滚到上一个内核
rollback_to_previous() {
    log_info "Attempting to rollback to previous kernel..."
    
    # 获取当前内核
    local current_kernel=$(uname -r)
    
    # 获取可用内核列表（排除当前内核）
    local available_kernels=$(list_available_kernels | grep -v "$current_kernel" | head -1)
    
    if [[ -z "$available_kernels" ]]; then
        log_error "No alternative kernels found for rollback"
        exit 1
    fi
    
    local target_kernel=$(echo "$available_kernels" | head -1)
    log_info "Found alternative kernel: $target_kernel"
    
    # 设置默认引导项
    if [[ $(detect_bootloader) == "grub" ]]; then
        if command -v grub-set-default >/dev/null 2>&1; then
            # 找到内核的引导菜单项
            local menu_entry=$(grep -l "$target_kernel" /boot/grub/grub.cfg 2>/dev/null | head -1)
            if [[ -n "$menu_entry" ]]; then
                grub-set-default "$menu_entry"
                log_success "Set default boot entry to: $menu_entry"
            fi
        fi
    fi
    
    log_success "Rollback configured. Reboot to use kernel: $target_kernel"
}

# 安全回滚检查
safe_rollback_check() {
    local kernel_name=$1
    
    # 检查是否试图移除当前运行的内核
    if [[ "$kernel_name" == "$(uname -r)" ]]; then
        log_error "Cannot remove the currently running kernel: $kernel_name"
        log_error "Please boot into a different kernel first"
        exit 1
    fi
    
    # 检查是否有其他可用的内核
    local other_kernels=$(list_available_kernels | grep -v "$(uname -r)" | wc -l)
    if [[ $other_kernels -eq 0 ]]; then
        log_error "No other kernels available. Removing this kernel would make the system unbootable!"
        exit 1
    fi
    
    log_success "Safety checks passed for kernel: $kernel_name"
}

# 显示使用说明
usage() {
    cat << EOF
SwiKernel Rollback Helper Script

Usage: $0 [OPTIONS]

Options:
  -l, --list              List available installed kernels
  -c, --current           Show current kernel information
  -b, --backup            Create configuration backup
  -r, --remove KERNEL     Remove specific kernel
  -R, --rollback          Rollback to previous kernel
  -s, --safe-remove KERNEL Safe removal with checks
  -u, --update-bootloader Update bootloader configuration
  -h, --help              Show this help message

Examples:
  $0 --list                    # List all installed kernels
  $0 --remove 5.15.0-76-generic # Remove specific kernel
  $0 --rollback               # Rollback to previous kernel
  $0 --safe-remove 5.15.0-76-generic # Safe removal with checks
EOF
}

# 主函数
main() {
    local action=""
    local kernel_name=""
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -l|--list)
                list_available_kernels
                exit 0
                ;;
            -c|--current)
                show_current_kernel
                exit 0
                ;;
            -b|--backup)
                backup_configuration
                exit 0
                ;;
            -r|--remove)
                kernel_name="$2"
                action="remove"
                shift
                ;;
            -R|--rollback)
                action="rollback"
                ;;
            -s|--safe-remove)
                kernel_name="$2"
                action="safe_remove"
                shift
                ;;
            -u|--update-bootloader)
                update_bootloader
                exit 0
                ;;
            -h|--help)
                usage
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                usage
                exit 1
                ;;
        esac
        shift
    done
    
    # 检查root权限
    check_root
    
    # 执行请求的操作
    case "$action" in
        "remove")
            remove_kernel_package "$kernel_name"
            update_bootloader
            ;;
        "safe_remove")
            safe_rollback_check "$kernel_name"
            remove_kernel_package "$kernel_name"
            update_bootloader
            ;;
        "rollback")
            rollback_to_previous
            ;;
        "")
            log_info "No action specified. Showing current status:"
            show_current_kernel
            list_available_kernels
            ;;
        *)
            log_error "Unknown action: $action"
            usage
            exit 1
            ;;
    esac
}

# 脚本入口
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi