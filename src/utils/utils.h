// src/utils/utils.h
#ifndef UTILS_H
#define UTILS_H

#include "swikernel.h"

// 日志级别
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3,
    LOG_FATAL = 4
} LogLevel;

// 错误级别
typedef enum {
    ERROR_WARNING = 0,
    ERROR_ERROR = 1,
    ERROR_FATAL = 2
} ErrorLevel;

// 回滚操作类型
typedef enum {
    ACTION_RESTORE_FILE = 0,
    ACTION_REMOVE_FILE = 1,
    ACTION_RESTORE_CONFIG = 2,
    ACTION_REMOVE_KERNEL = 3
} RollbackAction;

// 进度条类型
typedef enum {
    PROGRESS_SIMPLE = 0,
    PROGRESS_BLOCK = 1
} ProgressBarType;

// 数据结构
typedef struct {
    char backup_path[256];
    char original_path[256];
} FileBackupData;

typedef struct {
    char file_path[256];
} FileRemoveData;

typedef struct {
    char backup_path[256];
    char original_path[256];
} ConfigBackupData;

typedef struct {
    char kernel_name[128];
} KernelRemoveData;

// 错误处理回调
typedef void (*ErrorHandler)(ErrorLevel level, const char *message);

// 日志函数
int logger_init(const char *filename, LogLevel level, int rotate);
void log_message(LogLevel level, const char *format, ...);
void logger_cleanup(void);
void logger_set_level(LogLevel level);
LogLevel logger_get_level(void);

// 错误处理和回滚
void set_error_handler(ErrorHandler handler);
int add_rollback_step(RollbackAction action, void *data, size_t data_size);
void execute_rollback(void);
void clear_rollback_stack(void);
void report_error(ErrorLevel level, const char *message, const char *file, int line);
void print_backtrace(void);

// 配置管理
int load_config(SwikernelConfig *config);
int save_config(const SwikernelConfig *config, const char *filename);
void set_default_config(SwikernelConfig *config);
char* trim_whitespace(char *str);

// 进度显示
void show_progress_bar(const char *label, int current, int total, ProgressBarType type);
void show_spinner(const char *label, int *counter);
void show_download_progress(const char *label, long long current, long long total, 
                        double speed, time_t start_time);
void show_multi_progress(const char **labels, const int *currents, const int *totals, int count);

#endif // UTILS_H