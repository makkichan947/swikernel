// src/utils/error_handler.c
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>
#include <signal.h>
#include "error_handler.h"
#include "logger.h"

#define MAX_ROLLBACK_STEPS 20
#define MAX_BACKTRACE_DEPTH 20

typedef struct RollbackStep {
    RollbackAction action;
    void *data;
    size_t data_size;
    struct RollbackStep *next;
} RollbackStep;

static RollbackStep *rollback_stack = NULL;
static int rollback_count = 0;
static ErrorHandler global_error_handler = NULL;

// 信号处理函数
void signal_handler(int sig) {
    log_message(LOG_FATAL, "Received signal %d (%s)", sig, strsignal(sig));
    
    // 打印堆栈跟踪
    print_backtrace();
    
    // 执行回滚
    execute_rollback();
    
    // 退出程序
    exit(1);
}

// 设置全局错误处理器
void set_error_handler(ErrorHandler handler) {
    global_error_handler = handler;
    
    // 设置信号处理
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
}

// 添加回滚步骤
int add_rollback_step(RollbackAction action, void *data, size_t data_size) {
    if (rollback_count >= MAX_ROLLBACK_STEPS) {
        log_message(LOG_ERROR, "Rollback stack overflow");
        return -1;
    }
    
    RollbackStep *step = malloc(sizeof(RollbackStep));
    if (!step) {
        log_message(LOG_ERROR, "Failed to allocate rollback step");
        return -1;
    }
    
    step->action = action;
    step->data = malloc(data_size);
    if (!step->data) {
        free(step);
        log_message(LOG_ERROR, "Failed to allocate rollback data");
        return -1;
    }
    
    memcpy(step->data, data, data_size);
    step->data_size = data_size;
    step->next = rollback_stack;
    rollback_stack = step;
    rollback_count++;
    
    log_message(LOG_DEBUG, "Added rollback step %d (action: %d)", rollback_count, action);
    return 0;
}

// 执行回滚
void execute_rollback(void) {
    if (rollback_count == 0) {
        log_message(LOG_DEBUG, "No rollback steps to execute");
        return;
    }
    
    log_message(LOG_INFO, "Executing rollback (%d steps)", rollback_count);
    
    RollbackStep *current = rollback_stack;
    int steps_executed = 0;
    
    while (current) {
        log_message(LOG_DEBUG, "Executing rollback step %d", steps_executed + 1);
        
        switch (current->action) {
            case ACTION_RESTORE_FILE:
                if (!restore_backup_file((FileBackupData*)current->data)) {
                    steps_executed++;
                }
                break;
            case ACTION_REMOVE_FILE:
                if (!remove_installed_file((FileRemoveData*)current->data)) {
                    steps_executed++;
                }
                break;
            case ACTION_RESTORE_CONFIG:
                if (!restore_kernel_config((ConfigBackupData*)current->data)) {
                    steps_executed++;
                }
                break;
            case ACTION_REMOVE_KERNEL:
                if (!remove_kernel_entry((KernelRemoveData*)current->data)) {
                    steps_executed++;
                }
                break;
            default:
                log_message(LOG_WARNING, "Unknown rollback action: %d", current->action);
                break;
        }
        
        RollbackStep *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    
    rollback_stack = NULL;
    rollback_count = 0;
    log_message(LOG_INFO, "Rollback completed (%d steps executed)", steps_executed);
}

// 清空回滚栈（操作成功时调用）
void clear_rollback_stack(void) {
    RollbackStep *current = rollback_stack;
    int steps_cleared = 0;
    
    while (current) {
        RollbackStep *next = current->next;
        free(current->data);
        free(current);
        current = next;
        steps_cleared++;
    }
    
    rollback_stack = NULL;
    rollback_count = 0;
    log_message(LOG_DEBUG, "Rollback stack cleared (%d steps)", steps_cleared);
}

// 文件备份回滚实现
int restore_backup_file(FileBackupData *data) {
    log_message(LOG_INFO, "Restoring backup: %s -> %s", data->backup_path, data->original_path);
    
    if (rename(data->backup_path, data->original_path) != 0) {
        log_message(LOG_ERROR, "Failed to restore backup: %s", data->backup_path);
        return -1;
    }
    
    return 0;
}

// 移除安装的文件
int remove_installed_file(FileRemoveData *data) {
    log_message(LOG_INFO, "Removing installed file: %s", data->file_path);
    
    if (remove(data->file_path) != 0) {
        log_message(LOG_ERROR, "Failed to remove file: %s", data->file_path);
        return -1;
    }
    
    return 0;
}

// 恢复内核配置
int restore_kernel_config(ConfigBackupData *data) {
    log_message(LOG_INFO, "Restoring kernel config from: %s", data->backup_path);
    
    // 这里应该实现具体的配置恢复逻辑
    char command[512];
    snprintf(command, sizeof(command), "cp -r %s/* %s/", data->backup_path, data->original_path);
    
    if (system(command) != 0) {
        log_message(LOG_ERROR, "Failed to restore kernel config");
        return -1;
    }
    
    return 0;
}

// 移除内核条目
int remove_kernel_entry(KernelRemoveData *data) {
    log_message(LOG_INFO, "Removing kernel entry: %s", data->kernel_name);
    
    // 从GRUB配置中移除内核条目
    char command[512];
    snprintf(command, sizeof(command), 
            "sed -i '/%s/d' /boot/grub/grub.cfg", data->kernel_name);
    
    if (system(command) != 0) {
        log_message(LOG_WARNING, "Failed to remove kernel from GRUB config");
    }
    
    return 0;
}

// 打印堆栈跟踪
void print_backtrace(void) {
    void *buffer[MAX_BACKTRACE_DEPTH];
    int size = backtrace(buffer, MAX_BACKTRACE_DEPTH);
    char **strings = backtrace_symbols(buffer, size);
    
    if (strings) {
        log_message(LOG_ERROR, "Backtrace:");
        for (int i = 0; i < size; i++) {
            log_message(LOG_ERROR, "  %s", strings[i]);
        }
        free(strings);
    }
}

// 报告错误
void report_error(ErrorLevel level, const char *message, const char *file, int line) {
    char formatted_message[1024];
    snprintf(formatted_message, sizeof(formatted_message), 
            "%s (at %s:%d)", message, file, line);
    
    switch (level) {
        case ERROR_WARNING:
            log_message(LOG_WARNING, "%s", formatted_message);
            break;
        case ERROR_ERROR:
            log_message(LOG_ERROR, "%s", formatted_message);
            break;
        case ERROR_FATAL:
            log_message(LOG_FATAL, "%s", formatted_message);
            if (global_error_handler) {
                global_error_handler(level, formatted_message);
            }
            break;
    }
}