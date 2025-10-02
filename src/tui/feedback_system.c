#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "feedback_system.h"
#include "logger.h"

// 全局反馈系统实例
FeedbackSystem g_feedback_system;

// 初始化反馈系统
int feedback_system_init(FeedbackSystem *fs, FeedbackLevel level) {
    if (!fs) {
        return SWK_ERROR_INVALID_PARAM;
    }

    memset(fs, 0, sizeof(FeedbackSystem));
    fs->level = level;
    fs->max_messages = 100;
    fs->auto_display = 1;
    fs->sound_enabled = 0;
    fs->log_enabled = 1;
    strcpy(fs->sound_command, "echo -e '\\a'"); // 默认铃声

    // 初始化互斥锁
    if (pthread_mutex_init(&fs->mutex, NULL) != 0) {
        return SWK_ERROR_SYSTEM_CALL;
    }

    log_message(LOG_DEBUG, "Feedback system initialized (level: %d)", level);
    return SWK_SUCCESS;
}

// 清理反馈系统
void feedback_system_cleanup(FeedbackSystem *fs) {
    if (!fs) return;

    pthread_mutex_lock(&fs->mutex);

    // 清理消息队列
    FeedbackMessage *current = fs->messages;
    while (current) {
        FeedbackMessage *next = current->next;
        free(current);
        current = next;
    }

    fs->messages = NULL;
    fs->current_message = NULL;
    fs->message_count = 0;

    pthread_mutex_unlock(&fs->mutex);
    pthread_mutex_destroy(&fs->mutex);

    log_message(LOG_DEBUG, "Feedback system cleaned up");
}

// 设置反馈级别
int feedback_system_set_level(FeedbackSystem *fs, FeedbackLevel level) {
    if (!fs) {
        return SWK_ERROR_INVALID_PARAM;
    }

    pthread_mutex_lock(&fs->mutex);
    fs->level = level;
    pthread_mutex_unlock(&fs->mutex);

    log_message(LOG_DEBUG, "Feedback level changed to %d", level);
    return SWK_SUCCESS;
}

// 获取反馈级别
FeedbackLevel feedback_system_get_level(FeedbackSystem *fs) {
    if (!fs) return FEEDBACK_LEVEL_NORMAL;
    return fs->level;
}

// 添加反馈消息
int feedback_system_add_message(FeedbackSystem *fs, FeedbackType type, const char *title,
                               const char *message, const char *details) {
    if (!fs || !title || !message) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 根据反馈级别过滤消息
    if (type == FEEDBACK_TYPE_DEBUG && fs->level < FEEDBACK_LEVEL_DEBUG) {
        return SWK_SUCCESS; // 忽略调试消息
    }

    FeedbackMessage *msg = malloc(sizeof(FeedbackMessage));
    if (!msg) {
        return SWK_ERROR_OUT_OF_MEMORY;
    }

    memset(msg, 0, sizeof(FeedbackMessage));
    msg->type = type;
    msg->timestamp = time(NULL);
    msg->display_duration = 5; // 默认显示5秒
    msg->priority = type; // 优先级基于类型

    strncpy(msg->title, title, sizeof(msg->title) - 1);
    strncpy(msg->message, message, sizeof(msg->message) - 1);
    if (details) {
        strncpy(msg->details, details, sizeof(msg->details) - 1);
    }

    pthread_mutex_lock(&fs->mutex);

    // 添加到消息队列
    msg->next = fs->messages;
    fs->messages = msg;
    fs->message_count++;

    // 限制消息数量
    if (fs->message_count > fs->max_messages) {
        // 删除最旧的消息
        FeedbackMessage *current = fs->messages;
        FeedbackMessage *prev = NULL;

        while (current->next) {
            prev = current;
            current = current->next;
        }

        if (prev) {
            prev->next = NULL;
        }
        free(current);
        fs->message_count--;
    }

    pthread_mutex_unlock(&fs->mutex);

    // 记录到日志
    if (fs->log_enabled) {
        LogLevel log_level = LOG_INFO;
        switch (type) {
            case FEEDBACK_TYPE_DEBUG: log_level = LOG_DEBUG; break;
            case FEEDBACK_TYPE_INFO: log_level = LOG_INFO; break;
            case FEEDBACK_TYPE_SUCCESS: log_level = LOG_INFO; break;
            case FEEDBACK_TYPE_WARNING: log_level = LOG_WARNING; break;
            case FEEDBACK_TYPE_ERROR: log_level = LOG_ERROR; break;
            case FEEDBACK_TYPE_CRITICAL: log_level = LOG_FATAL; break;
        }

        log_message(log_level, "[%s] %s", title, message);
    }

    // 播放声音提示
    if (fs->sound_enabled && (type == FEEDBACK_TYPE_ERROR || type == FEEDBACK_TYPE_CRITICAL)) {
        system(fs->sound_command);
    }

    return SWK_SUCCESS;
}

// 显示反馈消息
int feedback_system_show_message(FeedbackSystem *fs, FeedbackType type, const char *title,
                                const char *message, const char *details) {
    if (feedback_system_add_message(fs, type, title, message, details) != SWK_SUCCESS) {
        return SWK_ERROR;
    }

    // 如果启用了自动显示，立即显示消息
    if (fs && fs->auto_display) {
        // 这里可以实现立即显示逻辑
        // 由于dialog库的限制，我们主要通过日志和返回值来反馈
    }

    return SWK_SUCCESS;
}

// 便捷反馈函数实现
int feedback_info(FeedbackSystem *fs, const char *title, const char *message, const char *details) {
    return feedback_system_show_message(fs, FEEDBACK_TYPE_INFO, title, message, details);
}

int feedback_success(FeedbackSystem *fs, const char *title, const char *message, const char *details) {
    return feedback_system_show_message(fs, FEEDBACK_TYPE_SUCCESS, title, message, details);
}

int feedback_warning(FeedbackSystem *fs, const char *title, const char *message, const char *details) {
    return feedback_system_show_message(fs, FEEDBACK_TYPE_WARNING, title, message, details);
}

int feedback_error(FeedbackSystem *fs, const char *title, const char *message, const char *details) {
    return feedback_system_show_message(fs, FEEDBACK_TYPE_ERROR, title, message, details);
}

int feedback_critical(FeedbackSystem *fs, const char *title, const char *message, const char *details) {
    return feedback_system_show_message(fs, FEEDBACK_TYPE_CRITICAL, title, message, details);
}

int feedback_debug(FeedbackSystem *fs, const char *title, const char *message, const char *details) {
    return feedback_system_show_message(fs, FEEDBACK_TYPE_DEBUG, title, message, details);
}

// 获取错误建议
const char *feedback_system_get_error_suggestion(SwkReturnCode code) {
    switch (code) {
        case SWK_ERROR_FILE_NOT_FOUND:
            return "请检查文件路径是否正确，并确认文件存在。";
        case SWK_ERROR_PERMISSION_DENIED:
            return "请检查文件权限，可能需要管理员权限。";
        case SWK_ERROR_OUT_OF_MEMORY:
            return "系统内存不足，请关闭其他程序后重试。";
        case SWK_ERROR_INVALID_PARAM:
            return "输入参数无效，请检查输入内容。";
        case SWK_ERROR_SYSTEM_CALL:
            return "系统调用失败，请检查系统状态。";
        case SWK_ERROR_COMPILATION:
            return "编译失败，请检查编译环境和依赖。";
        case SWK_ERROR_DEPENDENCY:
            return "依赖项缺失，请安装必要的软件包。";
        default:
            return "请检查系统配置和环境。";
    }
}

// 处理错误信息
int feedback_system_handle_error(FeedbackSystem *fs, ErrorInfo *error) {
    if (!fs || !error) {
        return SWK_ERROR_INVALID_PARAM;
    }

    char message[512];
    snprintf(message, sizeof(message), "%s (在 %s:%d)",
             error->message, error->function, error->line);

    const char *suggestion = feedback_system_get_error_suggestion(error->code);

    return feedback_error(fs, "操作失败", message, suggestion);
}

// 显示错误详情对话框
void show_error_details_dialog(ErrorInfo *error) {
    if (!error) return;

    char details_text[MAX_BUFFER_SIZE] = {0};
    snprintf(details_text, sizeof(details_text),
            "错误详情\n"
            "==========\n\n"
            "错误代码: %d\n"
            "错误信息: %s\n"
            "函数: %s\n"
            "文件: %s\n"
            "行号: %d\n\n"
            "建议解决方案:\n%s",
            error->code,
            error->message,
            error->function,
            error->file,
            error->line,
            feedback_system_get_error_suggestion(error->code));

    dialog_msgbox("错误详情", details_text, 20, 80);
}

// 清空消息
int feedback_system_clear_messages(FeedbackSystem *fs) {
    if (!fs) {
        return SWK_ERROR_INVALID_PARAM;
    }

    pthread_mutex_lock(&fs->mutex);

    FeedbackMessage *current = fs->messages;
    while (current) {
        FeedbackMessage *next = current->next;
        free(current);
        current = next;
    }

    fs->messages = NULL;
    fs->current_message = NULL;
    fs->message_count = 0;

    pthread_mutex_unlock(&fs->mutex);

    return SWK_SUCCESS;
}

// 获取消息列表
FeedbackMessage *feedback_system_get_messages(FeedbackSystem *fs) {
    if (!fs) return NULL;
    return fs->messages;
}

// 显示进度信息
int feedback_system_show_progress(FeedbackSystem *fs, ProgressInfo *progress) {
    if (!fs || !progress) {
        return SWK_ERROR_INVALID_PARAM;
    }

    progress->start_time = time(NULL);

    // 这里可以实现进度显示逻辑
    log_message(LOG_INFO, "Starting operation: %s", progress->operation);

    return SWK_SUCCESS;
}

// 更新进度信息
int feedback_system_update_progress(FeedbackSystem *fs, int current, const char *status) {
    if (!fs) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 这里可以实现进度更新显示逻辑
    if (status) {
        log_message(LOG_DEBUG, "Progress update: %d, %s", current, status);
    }

    return SWK_SUCCESS;
}

// 隐藏进度显示
int feedback_system_hide_progress(FeedbackSystem *fs) {
    if (!fs) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 这里可以实现隐藏进度显示逻辑
    log_message(LOG_DEBUG, "Progress display hidden");

    return SWK_SUCCESS;
}

// 显示状态信息
int feedback_system_show_status(FeedbackSystem *fs, const char *status) {
    if (!fs || !status) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 这里可以实现状态栏显示逻辑
    log_message(LOG_DEBUG, "Status: %s", status);

    return SWK_SUCCESS;
}

// 更新状态信息
int feedback_system_update_status(FeedbackSystem *fs, const char *status) {
    if (!fs || !status) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 这里可以实现状态更新逻辑
    log_message(LOG_DEBUG, "Status update: %s", status);

    return SWK_SUCCESS;
}

// 隐藏状态显示
int feedback_system_hide_status(FeedbackSystem *fs) {
    if (!fs) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 这里可以实现隐藏状态显示逻辑
    return SWK_SUCCESS;
}

// 确认对话框
int feedback_system_confirm(FeedbackSystem *fs, const char *title, const char *message,
                           const char *details, int default_yes) {
    if (!title || !message) {
        return 0;
    }

    char confirm_msg[MAX_BUFFER_SIZE] = {0};
    snprintf(confirm_msg, sizeof(confirm_msg), "%s\n\n%s",
             message, details ? details : "");

    int result = dialog_yesno((char*)title, confirm_msg, 12, 60);

    // 记录用户选择
    if (fs) {
        feedback_debug(fs, "User Confirmation",
                      result == 0 ? "User confirmed" : "User cancelled",
                      title);
    }

    return (result == 0); // 0表示确认，1表示取消
}

// 输入对话框
int feedback_system_input(FeedbackSystem *fs, const char *title, const char *prompt,
                         char *buffer, size_t buffer_size, int hidden) {
    if (!title || !prompt || !buffer) {
        return SWK_ERROR_INVALID_PARAM;
    }

    int result = dialog_inputbox((char*)title, (char*)prompt, 10, 60,
                                "", buffer, buffer_size);

    // 记录输入操作
    if (fs && result == 0) {
        feedback_debug(fs, "User Input", "Input received", title);
    }

    return (result == 0) ? SWK_SUCCESS : SWK_ERROR;
}

// 显示反馈对话框
void show_feedback_dialog(FeedbackSystem *fs) {
    if (!fs) return;

    pthread_mutex_lock(&fs->mutex);

    if (!fs->messages) {
        dialog_msgbox("反馈信息", "暂无反馈消息", 8, 50);
        pthread_mutex_unlock(&fs->mutex);
        return;
    }

    // 构建消息列表
    char display_text[MAX_BUFFER_SIZE] = {0};
    snprintf(display_text, sizeof(display_text), "反馈消息 (%d条)\n\n", fs->message_count);

    FeedbackMessage *current = fs->messages;
    int count = 0;
    char msg_line[256];

    while (current && count < 10) {
        char *type_str = "未知";
        switch (current->type) {
            case FEEDBACK_TYPE_INFO: type_str = "信息"; break;
            case FEEDBACK_TYPE_SUCCESS: type_str = "成功"; break;
            case FEEDBACK_TYPE_WARNING: type_str = "警告"; break;
            case FEEDBACK_TYPE_ERROR: type_str = "错误"; break;
            case FEEDBACK_TYPE_CRITICAL: type_str = "严重"; break;
            case FEEDBACK_TYPE_DEBUG: type_str = "调试"; break;
        }

        struct tm *tm = localtime(&current->timestamp);
        snprintf(msg_line, sizeof(msg_line), "%s [%02d:%02d:%02d] %s: %s\n",
                type_str,
                tm->tm_hour, tm->tm_min, tm->tm_sec,
                current->title,
                current->message);

        strncat(display_text, msg_line, sizeof(display_text) - strlen(display_text) - 1);
        current = current->next;
        count++;
    }

    pthread_mutex_unlock(&fs->mutex);

    dialog_msgbox("反馈消息", display_text, 20, 80);
}

// 显示进度对话框
void show_progress_dialog(ProgressInfo *progress) {
    if (!progress) return;

    char progress_text[MAX_BUFFER_SIZE] = {0};
    snprintf(progress_text, sizeof(progress_text),
            "操作进度\n"
            "==========\n\n"
            "操作: %s\n"
            "进度: %d/%d (%.1f%%)\n"
            "状态: %s\n",
            progress->operation,
            progress->current,
            progress->total,
            progress->total > 0 ? (double)progress->current * 100.0 / progress->total : 0.0,
            progress->status);

    // 计算ETA
    if (progress->show_eta && progress->current > 0 && progress->total > 0) {
        time_t elapsed = time(NULL) - progress->start_time;
        time_t remaining = (time_t)((double)elapsed * (progress->total - progress->current) / progress->current);

        snprintf(progress_text + strlen(progress_text), sizeof(progress_text) - strlen(progress_text),
                "预计剩余时间: %ld秒\n", remaining);
    }

    dialog_msgbox("进度", progress_text, 12, 60);
}

// 显示确认对话框
void show_confirmation_dialog(const char *title, const char *message, int *result) {
    if (!result) return;

    char confirm_msg[MAX_BUFFER_SIZE] = {0};
    snprintf(confirm_msg, sizeof(confirm_msg), "%s", message);

    *result = dialog_yesno((char*)title, confirm_msg, 10, 60);
}

// 设置声音命令
void feedback_system_set_sound_command(FeedbackSystem *fs, const char *command) {
    if (!fs || !command) return;

    pthread_mutex_lock(&fs->mutex);
    strncpy(fs->sound_command, command, sizeof(fs->sound_command) - 1);
    pthread_mutex_unlock(&fs->mutex);
}

// 启用/禁用声音
void feedback_system_enable_sound(FeedbackSystem *fs, int enable) {
    if (!fs) return;

    pthread_mutex_lock(&fs->mutex);
    fs->sound_enabled = enable;
    pthread_mutex_unlock(&fs->mutex);
}

// 设置语言
int feedback_system_set_language(FeedbackSystem *fs, const char *language) {
    if (!fs || !language) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 这里可以实现国际化逻辑
    log_message(LOG_DEBUG, "Language set to: %s", language);
    return SWK_SUCCESS;
}

// 获取国际化消息
const char *feedback_system_get_message(FeedbackSystem *fs, const char *key) {
    if (!key) return "";

    // 这里可以实现消息翻译逻辑
    // 目前返回原始键值
    return key;
}