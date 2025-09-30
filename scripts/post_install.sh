#!/bin/bash

# SwiKernel Post-Installation Setup Script

set -e

# 颜色定义
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
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

# 配置目录结构
setup_directories() {
    log_info "Setting up directory structure..."
    
    local directories=(
        "/etc/swikernel"
        "/var/log/swikernel"
        "/var/lib/swikernel/backups"
        "/var/lib/swikernel/cache"
        "/usr/local/share/swikernel/scripts"
    )
    
    for dir in "${directories[@]}"; do
        mkdir -p "$dir"
        chmod 755 "$(dirname "$dir")"
    done
    
    # 设置特定目录权限
    chmod 700 "/var/lib/swikernel"
    chmod 755 "/var/lib/swikernel/backups"
    chmod 755 "/var/lib/swikernel/cache"
    
    log_success "Directory structure created"
}

# 安装配置文件
install_configs() {
    log_info "Installing configuration files..."
    
    local config_source="./config/swikernel.conf"
    local config_target="/etc/swikernel/swikernel.conf"
    
    if [[ -f "$config_source" ]]; then
        cp "$config_source" "$config_target"
        chmod 644 "$config_target"
        log_success "Configuration file installed: $config_target"
    else
        log_warning "Source configuration file not found: $config_source"
        log_info "Creating default configuration..."
        
        cat > "$config_target" << 'EOF'
# SwiKernel Configuration File

[logging]
level = INFO
file = /var/log/swikernel.log
max_size = 10485760
rotate = true

[kernel]
default_source_dir = /usr/src
backup_enabled = true
auto_dependencies = true
parallel_compilation = true

[installation]
keep_source = false
auto_reboot = false
timeout = 3600

[ui]
color_scheme = dark
auto_complete = true
show_progress = true
EOF
        log_success "Default configuration created: $config_target"
    fi
}

# 安装系统服务
install_service() {
    log_info "Installing system service..."
    
    local service_file="/etc/systemd/system/swikernel.service"
    
    if [[ -d "/etc/systemd/system" ]]; then
        cat > "$service_file" << 'EOF'
[Unit]
Description=SwiKernel - Linux Kernel Switcher
Documentation=https://github.com/makkichan947/swikernel
After=network.target

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=/bin/true
ExecReload=/bin/true
StandardOutput=journal

[Install]
WantedBy=multi-user.target
EOF
        chmod 644 "$service_file"
        log_success "Systemd service installed"
    else
        log_warning "Systemd not available, skipping service installation"
    fi
}

# 设置日志轮转
setup_logrotate() {
    log_info "Setting up log rotation..."
    
    local logrotate_file="/etc/logrotate.d/swikernel"
    
    if [[ -d "/etc/logrotate.d" ]]; then
        cat > "$logrotate_file" << 'EOF'
/var/log/swikernel.log {
    daily
    missingok
    rotate 7
    compress
    delaycompress
    notifempty
    create 644 root root
}
EOF
        chmod 644 "$logrotate_file"
        log_success "Log rotation configured"
    else
        log_warning "Logrotate not available, skipping configuration"
    fi
}

# 安装Bash补全
install_bash_completion() {
    log_info "Installing bash completion..."
    
    local completion_source="./scripts/swikernel-completion.bash"
    local completion_target="/etc/bash_completion.d/swikernel"
    
    if [[ -d "/etc/bash_completion.d" ]]; then
        if [[ -f "$completion_source" ]]; then
            cp "$completion_source" "$completion_target"
            chmod 644 "$completion_target"
            log_success "Bash completion installed"
        else
            log_warning "Bash completion source not found, creating basic completion..."
            
            cat > "$completion_target" << 'EOF'
# Bash completion for SwiKernel
_swikernel_complete() {
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    opts="-h --help -S -list -version -verbose"
    
    if [[ ${cur} == -* ]]; then
        COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
        return 0
    fi
}
complete -F _swikernel_complete swikernel
EOF
            log_success "Basic bash completion created"
        fi
    else
        log_warning "Bash completion directory not found, skipping completion setup"
    fi
}

# 设置环境变量
setup_environment() {
    log_info "Setting up environment variables..."
    
    # 创建环境文件
    local env_file="/etc/swikernel/environment"
    
    cat > "$env_file" << EOF
# SwiKernel Environment Variables
export SWIKERNEL_CONFIG="/etc/swikernel/swikernel.conf"
export SWIKERNEL_LOG_DIR="/var/log/swikernel"
export SWIKERNEL_DATA_DIR="/var/lib/swikernel"
export PATH="\$PATH:/usr/local/share/swikernel/scripts"
EOF
    chmod 644 "$env_file"
    
    log_success "Environment variables configured"
}

# 验证安装
verify_installation() {
    log_info "Verifying installation..."
    
    local errors=0
    
    # 检查目录
    local required_dirs=(
        "/etc/swikernel"
        "/var/log/swikernel"
        "/var/lib/swikernel/backups"
    )
    
    for dir in "${required_dirs[@]}"; do
        if [[ -d "$dir" ]]; then
            log_info "Directory exists: $dir"
        else
            log_warning "Directory missing: $dir"
            ((errors++))
        fi
    done
    
    # 检查配置文件
    if [[ -f "/etc/swikernel/swikernel.conf" ]]; then
        log_info "Configuration file exists"
    else
        log_warning "Configuration file missing"
        ((errors++))
    fi
    
    # 检查可执行文件
    if command -v swikernel >/dev/null 2>&1; then
        log_info "SwiKernel executable found"
    else
        log_warning "SwiKernel executable not in PATH"
    fi
    
    if [[ $errors -eq 0 ]]; then
        log_success "Post-installation verification completed successfully"
    else
        log_warning "Post-installation completed with $errors warnings"
    fi
}

# 显示完成信息
show_completion_message() {
    echo
    log_success "SwiKernel post-installation setup completed!"
    echo
    log_info "What's next:"
    echo "  1. Ensure /usr/local/bin is in your PATH"
    echo "  2. Source your shell profile or restart your terminal"
    echo "  3. Run 'swikernel --help' to verify installation"
    echo "  4. Configure /etc/swikernel/swikernel.conf if needed"
    echo
    log_info "Useful directories:"
    echo "  Configuration: /etc/swikernel/"
    echo "  Logs: /var/log/swikernel/"
    echo "  Data: /var/lib/swikernel/"
    echo
}

# 主函数
main() {
    log_info "Starting SwiKernel post-installation setup..."
    
    setup_directories
    install_configs
    install_service
    setup_logrotate
    install_bash_completion
    setup_environment
    verify_installation
    show_completion_message
    
    log_success "Post-installation setup completed!"
}

# 脚本入口
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    # 检查root权限
    if [[ $EUID -ne 0 ]]; then
        echo -e "${YELLOW}[WARNING]${NC} This script requires root privileges. Please run with sudo."
        exit 1
    fi
    
    main "$@"
fi