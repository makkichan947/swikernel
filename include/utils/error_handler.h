#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "../common_defs.h"

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

// 函数声明
void set_error_handler(ErrorHandler handler);
int add_rollback_step(RollbackAction action, void *data, size_t data_size);
void execute_rollback(void);
void clear_rollback_stack(void);
void report_error(ErrorLevel level, const char *message, const char *file, int line);
void print_backtrace(void);

// 具体回滚函数
int restore_backup_file(FileBackupData *data);
int remove_installed_file(FileRemoveData *data);
int restore_kernel_config(ConfigBackupData *data);
int remove_kernel_entry(KernelRemoveData *data);

#endif