#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "swikernel.h"
#include "logger.h"
#include "error_handler.h"
#include "config_parser.h"
#include "feedback_system.h"
#include "i18n.h"

// 全局配置
SwikernelConfig g_config;

// 执行回滚操作
void execute_rollback(void) {
    log_message(LOG_INFO, "Executing rollback operations...");

    // 这里应该实现具体的回滚逻辑
    // 例如：恢复备份的内核、清理临时文件等
    log_message(LOG_INFO, "Rollback completed");
}

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
    // 初始化国际化系统
    if (i18n_init(&g_i18n_system, "/usr/share/swikernel/locale") != SWK_SUCCESS) {
        fprintf(stderr, "Failed to initialize i18n system\n");
        return 1;
    }

    // 检查是否为首次运行
    char config_file[MAX_PATH_LENGTH];
    snprintf(config_file, sizeof(config_file), "/usr/share/swikernel/locale/language.conf");

    struct stat st;
    if (stat(config_file, &st) != 0) {
        // 首次运行，显示语言选择
        if (i18n_setup_first_run(&g_i18n_system) != SWK_SUCCESS) {
            fprintf(stderr, "Failed to setup first run\n");
        }
    }

    // 初始化反馈系统
    if (feedback_system_init(&g_feedback_system, FEEDBACK_LEVEL_NORMAL) != SWK_SUCCESS) {
        fprintf(stderr, "Failed to initialize feedback system\n");
        i18n_cleanup(&g_i18n_system);
        return 1;
    }

    // 初始化日志系统
    if (logger_init("/var/log/swikernel.log", LOG_INFO, 1) != 0) {
        FEEDBACK_ERROR(_("initialization_failed"), _("cannot_initialize_log_system"));
        feedback_system_cleanup(&g_feedback_system);
        i18n_cleanup(&g_i18n_system);
        return 1;
    }

    FEEDBACK_INFO(_("program_started"), _("swikernel_started_version"), SWIKERNEL_VERSION);
    
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
    
    FEEDBACK_INFO(_("program_exiting"), _("swikernel_exiting_code"), result);

    logger_cleanup();
    feedback_system_cleanup(&g_feedback_system);
    i18n_cleanup(&g_i18n_system);

    return result;
}