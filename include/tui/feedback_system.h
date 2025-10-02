#ifndef FEEDBACK_SYSTEM_H
#define FEEDBACK_SYSTEM_H

#include "../common_defs.h"

// 反馈类型
typedef enum {
    FEEDBACK_TYPE_INFO,
    FEEDBACK_TYPE_SUCCESS,
    FEEDBACK_TYPE_WARNING,
    FEEDBACK_TYPE_ERROR,
    FEEDBACK_TYPE_CRITICAL,
    FEEDBACK_TYPE_DEBUG
} FeedbackType;

// 反馈级别
typedef enum {
    FEEDBACK_LEVEL_QUIET,      // 静默模式
    FEEDBACK_LEVEL_NORMAL,     // 正常模式
    FEEDBACK_LEVEL_VERBOSE,    // 详细模式
    FEEDBACK_LEVEL_DEBUG       // 调试模式
} FeedbackLevel;

// 反馈消息结构
typedef struct FeedbackMessage {
    FeedbackType type;
    char title[128];
    char message[512];
    char details[1024];
    time_t timestamp;
    int display_duration;      // 显示持续时间（秒）
    int priority;              // 优先级（数字越大优先级越高）
    struct FeedbackMessage *next;
} FeedbackMessage;

// 反馈系统状态
typedef struct {
    FeedbackMessage *messages;
    FeedbackMessage *current_message;
    int message_count;
    FeedbackLevel level;
    int max_messages;          // 最大消息数量
    int auto_display;          // 自动显示消息
    int sound_enabled;         // 启用声音提示
    int log_enabled;           // 启用日志记录
    char sound_command[256];   // 声音命令
    pthread_mutex_t mutex;     // 线程互斥锁
} FeedbackSystem;

// 反馈系统函数
int feedback_system_init(FeedbackSystem *fs, FeedbackLevel level);
void feedback_system_cleanup(FeedbackSystem *fs);
int feedback_system_set_level(FeedbackSystem *fs, FeedbackLevel level);
FeedbackLevel feedback_system_get_level(FeedbackSystem *fs);

// 消息管理
int feedback_system_add_message(FeedbackSystem *fs, FeedbackType type, const char *title,
                               const char *message, const char *details);
int feedback_system_show_message(FeedbackSystem *fs, FeedbackType type, const char *title,
                                const char *message, const char *details);
int feedback_system_clear_messages(FeedbackSystem *fs);
FeedbackMessage *feedback_system_get_messages(FeedbackSystem *fs);

// 便捷反馈函数
int feedback_info(FeedbackSystem *fs, const char *title, const char *message, const char *details);
int feedback_success(FeedbackSystem *fs, const char *title, const char *message, const char *details);
int feedback_warning(FeedbackSystem *fs, const char *title, const char *message, const char *details);
int feedback_error(FeedbackSystem *fs, const char *title, const char *message, const char *details);
int feedback_critical(FeedbackSystem *fs, const char *title, const char *message, const char *details);
int feedback_debug(FeedbackSystem *fs, const char *title, const char *message, const char *details);

// 进度反馈
typedef struct {
    char operation[128];
    int current;
    int total;
    char status[256];
    time_t start_time;
    int show_percentage;
    int show_eta;
} ProgressInfo;

int feedback_system_show_progress(FeedbackSystem *fs, ProgressInfo *progress);
int feedback_system_update_progress(FeedbackSystem *fs, int current, const char *status);
int feedback_system_hide_progress(FeedbackSystem *fs);

// 状态栏反馈
int feedback_system_show_status(FeedbackSystem *fs, const char *status);
int feedback_system_update_status(FeedbackSystem *fs, const char *status);
int feedback_system_hide_status(FeedbackSystem *fs);

// 确认对话框
int feedback_system_confirm(FeedbackSystem *fs, const char *title, const char *message,
                           const char *details, int default_yes);
int feedback_system_input(FeedbackSystem *fs, const char *title, const char *prompt,
                         char *buffer, size_t buffer_size, int hidden);

// 错误处理增强
typedef struct {
    SwkReturnCode code;
    char function[128];
    char file[256];
    int line;
    char message[512];
    char suggestion[512];
    void *context_data;
} ErrorInfo;

int feedback_system_handle_error(FeedbackSystem *fs, ErrorInfo *error);
int feedback_system_show_error_details(FeedbackSystem *fs, ErrorInfo *error);
const char *feedback_system_get_error_suggestion(SwkReturnCode code);

// TUI 反馈对话框
void show_feedback_dialog(FeedbackSystem *fs);
void show_error_details_dialog(ErrorInfo *error);
void show_progress_dialog(ProgressInfo *progress);
void show_confirmation_dialog(const char *title, const char *message, int *result);

// 反馈配置
int feedback_system_load_config(FeedbackSystem *fs, const char *config_file);
int feedback_system_save_config(FeedbackSystem *fs, const char *config_file);
void feedback_system_set_sound_command(FeedbackSystem *fs, const char *command);
void feedback_system_enable_sound(FeedbackSystem *fs, int enable);

// 国际化支持
int feedback_system_set_language(FeedbackSystem *fs, const char *language);
const char *feedback_system_get_message(FeedbackSystem *fs, const char *key);

// 全局反馈系统实例
extern FeedbackSystem g_feedback_system;

// 便捷宏定义
#define FEEDBACK_INFO(title, msg) feedback_info(&g_feedback_system, title, msg, NULL)
#define FEEDBACK_SUCCESS(title, msg) feedback_success(&g_feedback_system, title, msg, NULL)
#define FEEDBACK_WARNING(title, msg) feedback_warning(&g_feedback_system, title, msg, NULL)
#define FEEDBACK_ERROR(title, msg) feedback_error(&g_feedback_system, title, msg, NULL)
#define FEEDBACK_CRITICAL(title, msg) feedback_critical(&g_feedback_system, title, msg, NULL)
#define FEEDBACK_DEBUG(title, msg) feedback_debug(&g_feedback_system, title, msg, NULL)

#endif