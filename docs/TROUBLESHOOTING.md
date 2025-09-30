# SwiKernel 故障排除指南

## 目录
- [快速诊断](#快速诊断)
- [常见问题](#常见问题)
- [错误代码](#错误代码)
- [日志分析](#日志分析)
- [性能问题](#性能问题)
- [恢复步骤](#恢复步骤)
- [寻求帮助](#寻求帮助)

## 快速诊断

### 健康检查脚本
```bash
# 运行系统健康检查
sudo swikernel --diagnose

# 或使用独立脚本
curl -fsSL https://raw.githubusercontent.com/makkichan947/swikernel/main/scripts/diagnose.sh | bash
```

### 基本检查清单
1. **权限检查**: 确保有 root 权限或 sudo 访问
2. **磁盘空间**: 检查 /boot 和根分区空间
3. **内存检查**: 确保有足够内存进行编译
4. **网络连接**: 验证网络访问和 DNS 解析
5. **依赖验证**: 确认所有必要工具已安装

## 常见问题

### 安装问题

#### 1. 编译失败
**症状**: 编译过程中出现错误，构建终止

**解决方案**:
```bash
# 检查编译器版本
gcc --version
make --version

# 清理并重新构建
make distclean
make deps
make release

# 如果使用特定工具链
export CC=clang
export CXX=clang++
make release
```

**可能原因**:
- 编译器版本过旧或不兼容
- 缺少开发头文件
- 内存不足 (OOM killer)

#### 2. 依赖缺失
**症状**: 命令找不到或共享库错误

**解决方案**:
```bash
# 自动安装依赖
sudo ./scripts/setup_deps.sh

# 手动检查特定依赖
ldd $(which swikernel)

# 验证关键工具
for tool in gcc make dialog nasm; do
    which $tool || echo "Missing: $tool"
done
```

#### 3. 权限不足
**症状**: "Permission denied" 错误

**解决方案**:
```bash
# 使用 sudo
sudo swikernel [command]

# 或配置免密码 sudo
echo "$USER ALL=(ALL) NOPASSWD: /usr/local/bin/swikernel" | sudo tee /etc/sudoers.d/swikernel

# 检查当前用户组
groups $USER
```

### 运行时问题

#### 1. TUI 界面显示异常
**症状**: 界面乱码、颜色错误或布局混乱

**解决方案**:
```bash
# 检查终端设置
echo $TERM
echo $LANG

# 设置正确的终端类型
export TERM=xterm-256color

# 使用命令行模式
swikernel --no-tui --list-kernels

# 调整终端大小
stty rows 40 cols 120
```

#### 2. 内核列表为空
**症状**: `--list-kernels` 返回空列表

**解决方案**:
```bash
# 手动扫描内核
sudo find /boot -name "vmlinuz-*" | sed 's|/boot/vmlinuz-||'

# 检查包管理器
dpkg -l | grep linux-image  # Debian/Ubuntu
rpm -qa | grep kernel       # RHEL/CentOS/Fedora

# 更新内核缓存
sudo swikernel --update-cache
```

#### 3. 内核安装失败
**症状**: 安装过程中断或报错

**解决方案**:
```bash
# 检查磁盘空间
df -h /boot
df -h /var

# 检查依赖完整性
sudo swikernel --check-dependencies --fix

# 查看详细日志
tail -f /var/log/swikernel/install.log

# 尝试手动安装
cd /usr/src/linux
sudo make modules_install
sudo make install
```

### 引导问题

#### 1. 新内核无法启动
**症状**: 系统无法引导到新安装的内核

**解决方案**:
```bash
# 从旧内核启动后检查
sudo swikernel --list-kernels
sudo swikernel --set-default [old-kernel]

# 重新生成 initramfs
sudo update-initramfs -c -k $(uname -r)

# 更新引导加载器
sudo update-grub
# 或
sudo grub-mkconfig -o /boot/grub/grub.cfg
```

#### 2. 引导菜单缺失
**症状**: GRUB 菜单中看不到新内核

**解决方案**:
```bash
# 强制更新 GRUB
sudo update-grub

# 检查 GRUB 配置
sudo grep -r "linux" /boot/grub/

# 手动添加内核条目
sudo swikernel --add-boot-entry [kernel-version]
```

## 错误代码

### 系统错误 (1-99)
| 代码 | 描述 | 解决方案 |
|------|------|----------|
| 1 | 权限不足 | 使用 sudo 或检查用户组 |
| 2 | 磁盘空间不足 | 清理 /boot 或增加空间 |
| 3 | 内存不足 | 增加交换空间或减少编译作业 |
| 4 | 网络连接失败 | 检查网络设置和代理 |
| 5 | 文件系统只读 | 重新挂载为读写模式 |

### 编译错误 (100-199)
| 代码 | 描述 | 解决方案 |
|------|------|----------|
| 101 | 编译器未找到 | 安装 build-essential |
| 102 | 头文件缺失 | 安装 linux-headers |
| 103 | 链接失败 | 检查库路径和版本 |
| 104 | 配置错误 | 验证 .config 文件语法 |
| 105 | 模块编译失败 | 检查内核版本兼容性 |

### 安装错误 (200-299)
| 代码 | 描述 | 解决方案 |
|------|------|----------|
| 201 | 内核已存在 | 使用不同版本名称 |
| 202 | 引导更新失败 | 手动运行 update-grub |
| 203 | 模块安装失败 | 检查 /lib/modules 权限 |
| 204 | initramfs 创建失败 | 检查 dracut 或 mkinitcpio |
| 205 | 签名验证失败 | 禁用验证或导入密钥 |

### 配置错误 (300-399)
| 代码 | 描述 | 解决方案 |
|------|------|----------|
| 301 | 配置文件语法错误 | 验证 INI 格式 |
| 302 | 无效参数 | 检查命令行选项 |
| 303 | 依赖冲突 | 解决包依赖关系 |
| 304 | 架构不匹配 | 检查目标架构支持 |
| 305 | 版本不兼容 | 更新 SwiKernel 或内核 |

## 日志分析

### 日志文件位置
```bash
# 主日志文件
/var/log/swikernel/swikernel.log

# 安装日志
/var/log/swikernel/install.log

# 调试日志 (启用调试时)
/var/log/swikernel/debug.log

# 系统日志
journalctl -u swikernel
```

### 日志级别说明
```bash
# 查看当前日志级别
swikernel --get-log-level

# 设置日志级别
swikernel --set-log-level DEBUG

# 日志级别含义
# DEBUG: 详细调试信息
# INFO: 常规操作信息  
# WARNING: 警告信息
# ERROR: 错误信息
# FATAL: 致命错误
```

### 常见日志模式
```bash
# 跟踪特定操作
tail -f /var/log/swikernel/swikernel.log | grep "install"

# 搜索错误
grep -i error /var/log/swikernel/swikernel.log

# 分析性能
grep "duration" /var/log/swikernel/swikernel.log

# 检查时间戳
grep "^\[" /var/log/swikernel/swikernel.log | head -10
```

## 性能问题

### 编译性能优化

#### 1. 并行编译
```bash
# 使用所有 CPU 核心
export MAKEFLAGS="-j$(nproc)"

# 或指定核心数
export MAKEFLAGS="-j4"

# 在配置中设置
echo "make_jobs = 0" >> /etc/swikernel/swikernel.conf
```

#### 2. 内存优化
```bash
# 增加交换空间
sudo fallocate -l 2G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile

# 使用编译缓存
export CCACHE_DIR="/var/cache/ccache"
ccache -M 5G  # 设置 5GB 缓存
```

#### 3. 磁盘优化
```bash
# 使用 tmpfs 进行编译
sudo mount -t tmpfs -o size=8G tmpfs /usr/src/build

# 优化文件系统
sudo mount -o remount,noatime /
```

### 运行时性能

#### 1. 减少内存使用
```bash
# 调整缓存大小
echo "cache_size = 256" >> /etc/swikernel/swikernel.conf

# 限制并行任务
echo "parallel_tasks = 2" >> /etc/swikernel/swikernel.conf
```

#### 2. 优化启动时间
```bash
# 禁用不必要的服务
sudo systemctl disable swikernel-monitor

# 预加载内核缓存
sudo swikernel --preload-cache
```

## 恢复步骤

### 系统无法启动

#### 1. 从恢复模式启动
1. 在 GRUB 菜单中选择 "Advanced options"
2. 选择一个之前正常工作的内核
3. 选择 "Recovery mode"
4. 进入 root shell

#### 2. 恢复步骤
```bash
# 挂载系统为读写
mount -o remount,rw /

# 设置旧内核为默认
swikernel --set-default [old-kernel-version]

# 移除有问题的新内核
swikernel --remove [problematic-kernel]

# 更新引导
update-grub

# 重启系统
reboot
```

### 配置文件损坏

#### 1. 恢复默认配置
```bash
# 备份当前配置
cp /etc/swikernel/swikernel.conf /etc/swikernel/swikernel.conf.backup

# 恢复默认配置
swikernel --reset-config

# 或手动创建
cp /usr/share/swikernel/swikernel.conf.default /etc/swikernel/swikernel.conf
```

#### 2. 配置验证
```bash
# 检查配置语法
swikernel --validate-config

# 测试配置
swikernel --dry-run --list-kernels
```

### 数据损坏恢复

#### 1. 从备份恢复
```bash
# 列出可用备份
swikernel --list-backups

# 恢复特定备份
swikernel --restore-backup [backup-id]

# 恢复引导配置
swikernel --restore-grub
```

#### 2. 手动恢复
```bash
# 恢复内核文件
cp /var/lib/swikernel/backups/[date]/vmlinuz-* /boot/
cp /var/lib/swikernel/backups/[date]/initrd.img-* /boot/

# 恢复模块
cp -r /var/lib/swikernel/backups/[date]/lib/modules/* /lib/modules/

# 恢复 GRUB 配置
cp /var/lib/swikernel/backups/[date]/grub.cfg /boot/grub/
```

## 寻求帮助

### 收集诊断信息
```bash
# 生成诊断报告
swikernel --generate-report

# 报告将包含:
# - 系统信息
# - 安装详情  
# - 配置摘要
# - 最近日志
# - 错误统计
```

### 联系支持

#### GitHub Issues
- **地址**: https://github.com/makkichan947/swikernel/issues
- **要求**: 提供诊断报告和详细描述

#### 电子邮件
- **地址**: makkichan947@hotmail.com / nekosparry0727@outlook.com

### 提供有效信息

当请求帮助时，请提供:
1. **SwiKernel 版本**: `swikernel --version`
2. **系统信息**: `uname -a` 和 `lsb_release -a`
3. **错误消息**: 完整的错误输出
4. **相关日志**: 最后 100 行相关日志
5. **复现步骤**: 如何触发问题的详细步骤

### 已知限制

1. **架构支持**: 某些架构功能可能有限
2. **发行版**: 非主流发行版可能遇到兼容性问题  
3. **内核版本**: 非常旧或非常新的内核可能不受支持
4. **硬件**: 特定硬件可能需要额外配置

---

**提示**: 在尝试任何恢复操作前，请务必备份重要数据！