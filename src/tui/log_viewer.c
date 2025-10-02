#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include "log_viewer.h"
#include "logger.h"
#include "common_defs.h"

// 初始化日志查看器
int log_viewer_init(LogViewer *lv, const char *log_file_path) {
    if (!lv || !log_file_path) {
        return SWK_ERROR_INVALID_PARAM;
    }

    memset(lv, 0, sizeof(LogViewer));

    // 设置默认值
    lv->min_level = LOG_DEBUG;
    lv->show_timestamp = 1;
    lv->show_source_location = 1;
    lv->auto_refresh = 1;
    lv->follow_tail = 1;
    lv->visible_entries = 20;

    strncpy(lv->log_file_path, log_file_path, sizeof(lv->log_file_path) - 1);

    return log_viewer_reload(lv);
}

// 清理日志查看器
void log_viewer_cleanup(LogViewer *lv) {
    if (!lv) return;

    if (lv->log_file) {
        fclose(lv->log_file);
        lv->log_file = NULL;
    }

    // 释放日志条目
    LogEntry *current = lv->entries;
    while (current) {
        LogEntry *next = current->next;
        free(current);
        current = next;
    }

    lv->entries = NULL;
    lv->current_entry = NULL;
    lv->total_entries = 0;
}

// 重新加载日志文件
int log_viewer_reload(LogViewer *lv) {
    if (!lv) {
        return SWK_ERROR_INVALID_PARAM;
    }

    FILE *file = fopen(lv->log_file_path, "r");
    if (!file) {
        log_message(LOG_ERROR, "Cannot open log file: %s", lv->log_file_path);
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    // 关闭旧文件
    if (lv->log_file) {
        fclose(lv->log_file);
    }

    lv->log_file = file;

    // 获取文件大小
    struct stat statbuf;
    if (stat(lv->log_file_path, &statbuf) == 0) {
        lv->last_file_size = statbuf.st_size;
    }

    // 清空现有条目
    log_viewer_cleanup(lv);

    // 读取所有日志条目
    char line[2048];
    LogEntry *tail = NULL;

    while (fgets(line, sizeof(line), file)) {
        // 移除换行符
        line[strcspn(line, "\n\r")] = '\0';

        if (strlen(line) == 0) continue;

        LogEntry *entry = parse_log_line(line);
        if (entry) {
            // 添加到链表
            if (!lv->entries) {
                lv->entries = entry;
                tail = entry;
            } else {
                tail->next = entry;
                entry->prev = tail;
                tail = entry;
            }

            lv->total_entries++;
        }
    }

    // 设置当前条目为最后一个
    lv->current_entry = tail;
    lv->current_position = lv->total_entries - 1;

    log_message(LOG_DEBUG, "Loaded %d log entries from %s", lv->total_entries, lv->log_file_path);
    return SWK_SUCCESS;
}

// 刷新日志（检查新内容）
int log_viewer_refresh(LogViewer *lv) {
    if (!lv || !lv->log_file) {
        return SWK_ERROR_INVALID_PARAM;
    }

    struct stat statbuf;
    if (stat(lv->log_file_path, &statbuf) != 0) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    // 检查文件是否有变化
    if (statbuf.st_size == lv->last_file_size) {
        return SWK_SUCCESS; // 无变化
    }

    long current_pos = ftell(lv->log_file);
    fseek(lv->log_file, lv->last_file_size, SEEK_SET);

    char line[2048];
    LogEntry *tail = lv->current_entry;

    while (fgets(line, sizeof(line), lv->log_file)) {
        line[strcspn(line, "\n\r")] = '\0';

        if (strlen(line) == 0) continue;

        LogEntry *entry = parse_log_line(line);
        if (entry) {
            if (!lv->entries) {
                lv->entries = entry;
                tail = entry;
            } else {
                tail->next = entry;
                entry->prev = tail;
                tail = entry;
            }

            lv->total_entries++;
        }
    }

    lv->last_file_size = statbuf.st_size;

    // 如果跟随尾部模式，滚动到最新条目
    if (lv->follow_tail) {
        lv->current_entry = tail;
        lv->current_position = lv->total_entries - 1;
    }

    fseek(lv->log_file, current_pos, SEEK_SET);
    return SWK_SUCCESS;
}

// 解析日志行
LogEntry *parse_log_line(const char *line) {
    if (!line) return NULL;

    LogEntry *entry = malloc(sizeof(LogEntry));
    if (!entry) return NULL;

    memset(entry, 0, sizeof(LogEntry));

    // 尝试匹配标准日志格式：[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message
    char *bracket1 = strchr(line, '[');
    char *bracket2 = bracket1 ? strchr(bracket1 + 1, ']') : NULL;
    char *bracket3 = bracket2 ? strchr(bracket2 + 1, '[') : NULL;
    char *bracket4 = bracket3 ? strchr(bracket3 + 1, ']') : NULL;

    if (bracket1 && bracket2 && bracket3 && bracket4) {
        // 提取时间戳
        size_t timestamp_len = bracket2 - bracket1 - 1;
        if (timestamp_len < sizeof(entry->timestamp)) {
            strncpy(entry->timestamp, bracket1 + 1, timestamp_len);
            entry->timestamp[timestamp_len] = '\0';
        }

        // 提取日志级别
        size_t level_len = bracket4 - bracket3 - 1;
        if (level_len < 16) {
            char level_str[16];
            strncpy(level_str, bracket3 + 1, level_len);
            level_str[level_len] = '\0';
            entry->level = parse_log_level(level_str);
        }

        // 提取消息
        char *message_start = bracket4 + 1;
        while (*message_start == ' ') message_start++;
        strncpy(entry->message, message_start, sizeof(entry->message) - 1);
    } else {
        // 无法解析，使用整行作为消息
        strncpy(entry->message, line, sizeof(entry->message) - 1);
        strcpy(entry->timestamp, "Unknown");
        entry->level = LOG_INFO;
    }

    return entry;
}

// 解析日志级别字符串
LogLevel parse_log_level(const char *level_str) {
    if (!level_str) return LOG_INFO;

    if (strcasecmp(level_str, "DEBUG") == 0) return LOG_DEBUG;
    if (strcasecmp(level_str, "INFO") == 0) return LOG_INFO;
    if (strcasecmp(level_str, "WARN") == 0 || strcasecmp(level_str, "WARNING") == 0) return LOG_WARNING;
    if (strcasecmp(level_str, "ERROR") == 0) return LOG_ERROR;
    if (strcasecmp(level_str, "FATAL") == 0) return LOG_FATAL;

    return LOG_INFO;
}

// 日志级别转字符串
const char *level_to_string(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

// 日志级别转颜色代码
const char *level_to_color(LogLevel level) {
    switch (level) {
        case LOG_DEBUG: return "\033[36m"; // 青色
        case LOG_INFO: return "\033[32m";  // 绿色
        case LOG_WARNING: return "\033[33m"; // 黄色
        case LOG_ERROR: return "\033[31m"; // 红色
        case LOG_FATAL: return "\033[35m"; // 紫色
        default: return "\033[0m"; // 重置
    }
}

// 设置级别过滤器
void log_viewer_set_level_filter(LogViewer *lv, LogLevel min_level) {
    if (!lv) return;
    lv->min_level = min_level;
}

// 设置模式过滤器
void log_viewer_set_pattern_filter(LogViewer *lv, const char *pattern) {
    if (!lv || !pattern) return;

    strncpy(lv->filter_pattern, pattern, sizeof(lv->filter_pattern) - 1);
}

// 设置来源过滤器
void log_viewer_set_source_filter(LogViewer *lv, const char *source) {
    if (!lv || !source) return;

    strncpy(lv->source_filter, source, sizeof(lv->source_filter) - 1);
}

// 清除所有过滤器
void log_viewer_clear_filters(LogViewer *lv) {
    if (!lv) return;

    lv->min_level = LOG_DEBUG;
    lv->filter_pattern[0] = '\0';
    lv->source_filter[0] = '\0';
}

// 获取当前条目
LogEntry *log_viewer_get_current_entry(LogViewer *lv) {
    if (!lv) return NULL;
    return lv->current_entry;
}

// 向上滚动
int log_viewer_scroll_up(LogViewer *lv, int lines) {
    if (!lv || !lv->current_entry) return SWK_ERROR_INVALID_PARAM;

    for (int i = 0; i < lines && lv->current_entry->prev; i++) {
        lv->current_entry = lv->current_entry->prev;
        lv->current_position--;
    }

    return SWK_SUCCESS;
}

// 向下滚动
int log_viewer_scroll_down(LogViewer *lv, int lines) {
    if (!lv || !lv->current_entry) return SWK_ERROR_INVALID_PARAM;

    for (int i = 0; i < lines && lv->current_entry->next; i++) {
        lv->current_entry = lv->current_entry->next;
        lv->current_position++;
    }

    return SWK_SUCCESS;
}

// 滚动到顶部
int log_viewer_scroll_to_top(LogViewer *lv) {
    if (!lv || !lv->entries) return SWK_ERROR_INVALID_PARAM;

    lv->current_entry = lv->entries;
    lv->current_position = 0;
    return SWK_SUCCESS;
}

// 滚动到底部
int log_viewer_scroll_to_bottom(LogViewer *lv) {
    if (!lv || !lv->entries) return SWK_ERROR_INVALID_PARAM;

    while (lv->current_entry->next) {
        lv->current_entry = lv->current_entry->next;
        lv->current_position++;
    }

    return SWK_SUCCESS;
}

// 向上翻页
int log_viewer_page_up(LogViewer *lv) {
    return log_viewer_scroll_up(lv, lv->visible_entries);
}

// 向下翻页
int log_viewer_page_down(LogViewer *lv) {
    return log_viewer_scroll_down(lv, lv->visible_entries);
}

// 获取总条目数
int log_viewer_get_total_entries(LogViewer *lv) {
    if (!lv) return 0;
    return lv->total_entries;
}

// 获取按级别过滤的条目数
int log_viewer_get_entries_by_level(LogViewer *lv, LogLevel level) {
    if (!lv) return 0;

    int count = 0;
    LogEntry *current = lv->entries;

    while (current) {
        if (current->level == level) count++;
        current = current->next;
    }

    return count;
}

// 切换时间戳显示
void log_viewer_toggle_timestamp(LogViewer *lv) {
    if (!lv) return;
    lv->show_timestamp = !lv->show_timestamp;
}

// 切换源位置显示
void log_viewer_toggle_source_location(LogViewer *lv) {
    if (!lv) return;
    lv->show_source_location = !lv->show_source_location;
}

// 切换自动刷新
void log_viewer_toggle_auto_refresh(LogViewer *lv) {
    if (!lv) return;
    lv->auto_refresh = !lv->auto_refresh;
}

// 切换跟随尾部
void log_viewer_toggle_follow_tail(LogViewer *lv) {
    if (!lv) return;
    lv->follow_tail = !lv->follow_tail;
}