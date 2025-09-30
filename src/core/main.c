#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "swikernel.h"
#include "logger.h"
#include "error_handler.h"
#include "config_parser.h"

// 全局配置
SwikernelConfig g_config;

// 信号处理函数
void signal_handler(int sig) {
    log_message(LOG_INFO, "Received signal %d, cleaning up...", sig);
    execute_rollback();
    exit(1);
}

// 命令行参数解析
int parse_arguments(int argc, char *argv[]) {
    if (argc == 1) {
        return MODE_TUI;  // 启动 TUI 界面
    }
    
    if (strcmp(argv[1], "-list") == 0) {
        return MODE_LIST_KERNELS;
    } else if (strcmp(argv[1], "-S") == 0 && argc == 3) {
        strncpy(g_config.install_kernel, argv[2], sizeof(g_config.install_kernel) - 1);
        return MODE_INSTALL_KERNEL;
    } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        return MODE_HELP;
    }
    
    return MODE_INVALID;
}

// 显示使用帮助
void show_usage(void) {
    printf("SwiKernel - Linux Kernel Switcher v%s\n", SWIKERNEL_VERSION);
    printf("Usage:\n");
    printf("  swikernel                    # Start TUI interface\n");
    printf("  swikernel -list             # List available kernels\n");
    printf("  swikernel -S <kernel-name>  # Install specific kernel\n");
    printf("  swikernel -h/--help         # Show this help\n");
}

int main(int argc, char *argv[]) {
    // 初始化日志系统
    if (logger_init("/var/log/swikernel.log", LOG_INFO, 1) != 0) {
        fprintf(stderr, "Failed to initialize logger\n");
        return 1;
    }
    
    log_message(LOG_INFO, "SwiKernel started (version: %s)", SWIKERNEL_VERSION);
    
    // 注册信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 加载配置
    if (load_config(&g_config) != 0) {
        log_message(LOG_WARNING, "Using default configuration");
        set_default_config(&g_config);
    }
    
    // 解析命令行参数
    int mode = parse_arguments(argc, argv);
    
    int result = 0;
    switch (mode) {
        case MODE_TUI:
            log_message(LOG_INFO, "Starting TUI mode");
            result = start_tui_interface();
            break;
            
        case MODE_LIST_KERNELS:
            log_message(LOG_INFO, "Listing available kernels");
            result = list_available_kernels();
            break;
            
        case MODE_INSTALL_KERNEL:
            log_message(LOG_INFO, "Installing kernel: %s", g_config.install_kernel);
            result = install_kernel_cli(g_config.install_kernel);
            break;
            
        case MODE_HELP:
            show_usage();
            break;
            
        case MODE_INVALID:
        default:
            log_message(LOG_ERROR, "Invalid arguments");
            show_usage();
            result = 1;
            break;
    }
    
    log_message(LOG_INFO, "SwiKernel exiting with code %d", result);
    logger_cleanup();
    
    return result;
}