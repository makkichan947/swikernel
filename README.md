<h1 align="center">SwiKernel - Linux Kernel Switcher</h1>

<p align="center">
  <strong>Simple, Safe, and Powerful Linux Kernel Management Tool</strong>
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
  <a href="#features">Features</a> •
  <a href="#quick-start">Quick Start</a> •
  <a href="#installation">Installation</a> •
  <a href="#usage-guide">Usage Guide</a> •
  <a href="#documentation">Documentation</a> •
  <a href="#contributing">Contributing</a> •
  <a href="#license">License</a>
</p>

```
███████╗██╗    ██╗██╗██╗  ██╗███████╗██████╗ ███╗   ██╗███████╗██╗
██╔════╝██║    ██║██║██║ ██╔╝██╔════╝██╔══██╗████╗  ██║██╔════╝██║
███████╗██║ █╗ ██║██║█████╔╝ █████╗  ██████╔╝██╔██╗ ██║█████╗  ██║
╚════██║██║███╗██║██║██╔═██╗ ██╔══╝  ██╔══██╗██║╚██╗██║██╔══╝  ██║
███████║╚███╔███╔╝██║██║  ██╗███████╗██║  ██║██║ ╚████║███████╗███████╗
╚══════╝ ╚══╝╚══╝ ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝╚══════╝
```

## 🌟 Features

### 🚀 Core Features
- **Multi-Kernel Management**: Easily install, switch, and remove multiple Linux kernels
- **One-Click Compilation**: Automatically compile and install kernels from source code
- **Smart Dependencies**: Automatically detect and install compilation dependencies
- **Safe Rollback**: Complete rollback mechanism to ensure system safety
- **Preset Configurations**: Optimized configuration presets for different use cases

### 🛡️ Security & Reliability
- **Signature Verification**: Automatically verify kernel signatures and integrity
- **Backup Protection**: Automatically backup system configuration before installation
- **Sandbox Mode**: Safe compilation and testing environment
- **Permission Control**: Fine-grained permission management and auditing

### 🎯 User Experience
- **Intuitive TUI**: Text-based user interface based on dialog
- **Command Line Interface**: Complete command-line tools
- **Auto-Completion**: Intelligent path and kernel name completion
- **Progress Display**: Real-time compilation and installation progress
- **Multi-Language Support**: Internationalization and localization

### 🔧 Advanced Features
- **Cross-Compilation**: Support for multi-architecture kernel compilation
- **Custom Configuration**: Flexible kernel configuration management
- **Module Management**: Intelligent kernel module management
- **Performance Optimization**: Hardware-optimized configurations
- **Plugin System**: Extensible plugin architecture
- **File Management**: Built-in file browser and operation tools
- **Log System**: Real-time log viewing and filtering capabilities
- **Configuration Interface**: Intuitive configuration options management interface
- **Performance Monitoring**: Real-time system performance dashboard
- **Theme System**: User-customizable color and appearance themes
- **Layout Management**: Adaptive screen layout and responsive design
- **Error Handling**: Enhanced error handling and user feedback system

## 🚀 Quick Start

### System Requirements
- **Operating System**: Linux (kernel 4.4+)
- **Architecture**: x86_64, arm64, arm, ppc64le, riscv64
- **Memory**: 2GB+ (4GB+ recommended for compilation)
- **Storage**: 10GB+ available space
- **Permissions**: Root access required

### 5-Minute Installation Experience

```bash
# 1. Download installation script
curl -fsSL https://raw.githubusercontent.com/makkichan947/swikernel/main/scripts/install.sh -o install.sh

# 2. Run installation
chmod +x install.sh
sudo ./install.sh

# 3. Start TUI interface
sudo swikernel
```

### Basic Usage Examples

```bash
# List available kernels
swikernel --list-kernels

# Install new kernel
sudo swikernel --install linux-6.1

# Install from source
sudo swikernel --install-from-source /path/to/kernel/source --name my-custom-kernel

# Switch active kernel
sudo swikernel --switch-to linux-6.1

# Remove old kernel
sudo swikernel --remove linux-5.15
```

## 📦 Installation

### Method 1: One-Click Installation (Recommended)
```bash
curl -fsSL https://raw.githubusercontent.com/makkichan947/swikernel/main/scripts/install.sh | bash
```

### Method 2: Compile from Source
```bash
git clone https://github.com/makkichan947/swikernel.git
cd swikernel
make deps
make release
sudo make install
```

## 📖 Usage Guide

### TUI Interface Usage

Start the text user interface:
```bash
sudo swikernel
```

Main menu options:
1. **Kernel Management** - View, install, and remove kernels
2. **Source Compilation** - Compile custom kernels from source
3. **System Settings** - Configure SwiKernel behavior
4. **Tool Collection** - Advanced tools and utilities
5. **File Manager** - Built-in file browser and operations
6. **Log Viewer** - Real-time log viewing and filtering
7. **Configuration** - Intuitive configuration management
8. **Plugin Manager** - Plugin system management
9. **System Monitor** - Real-time performance dashboard
10. **Theme Manager** - Custom themes and colors
11. **Layout Manager** - Responsive layout management

### Command Line Reference

#### Kernel Management
```bash
# List all available kernels
swikernel --list-kernels

# Install specific version kernel
sudo swikernel --install linux-6.1.28

# Install from source
sudo swikernel --install-from-source /usr/src/linux-6.1 --name custom-6.1

# Switch active kernel
sudo swikernel --switch-to linux-6.1.28

# Remove old kernel
sudo swikernel --remove linux-5.15.90

# Set default kernel
sudo swikernel --set-default linux-6.1.28
```

#### System Management
```bash
# Check system dependencies
swikernel --check-dependencies

# Update bootloader configuration
sudo swikernel --update-bootloader

# Clean cache and temporary files
swikernel --clean-cache

# Validate system status
swikernel --diagnose
```

#### Advanced Features
```bash
# Cross-compilation (ARM64)
sudo swikernel --cross-compile arm64 --install-from-source /path/to/source

# Use configuration presets
sudo swikernel --install linux-6.1 --preset server

# Custom compilation options
sudo swikernel --install-from-source /path/to/source --make-args="-j8"

# Generate debug report
swikernel --generate-report
```

### Configuration Management

SwiKernel's configuration file is located at `/etc/swikernel/swikernel.conf`:

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

### Configuration Presets

SwiKernel provides multiple kernel configuration presets:

- **minimal**: Minimal configuration, suitable for embedded systems
- **desktop**: Desktop optimization, supports multimedia and graphics
- **server**: Server optimization, emphasizes stability and performance
- **performance**: Maximum performance, suitable for workstations
- **security**: Security hardening, enhanced system security

Usage:
```bash
sudo swikernel --install linux-6.1 --preset server
```

## 📚 Documentation

### Detailed Documentation
- **[Installation Guide](docs/INSTALL.md)** - Complete installation instructions and troubleshooting
- **[Troubleshooting](docs/TROUBLESHOOTING.md)** - Common problems and solutions

### Getting Help
- **GitHub Issues**: [Report Issues](https://github.com/makkichan947/swikernel/issues)
- **Email**: makkichan947@hotmail.com / nekosparry0727@outlook.com
- **Stack Overflow**: Use tag `[swikernel]`

## 🤝 Contributing

We welcome contributions of all kinds!

### Reporting Issues
If you find bugs or have feature suggestions, please [create an Issue](https://github.com/makkichan947/swikernel/issues/new/choose).

### Code Contributions
1. Fork this project
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

### Development Environment Setup
```bash
# Clone repository
git clone https://github.com/makkichan947/swikernel.git
cd swikernel

# Install development dependencies
make deps-dev

# Build debug version
make debug

# Run tests
make test

# Code formatting check
make format-check
```

## 📄 License

This project is licensed under the **NC-OSL** license - see the [LICENSE](LICENSE) file for details.

---

<div align="center">

**If this project helps you, please give it a ⭐️ star!**

[![Star History Chart](https://api.star-history.com/svg?repos=makkichan947/swikernel&type=Date)](https://star-history.com/#makkichan947swikernel&Date)

</div>