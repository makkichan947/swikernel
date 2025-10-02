#ifndef LOG_VIEWER_H
#define LOG_VIEWER_H

#include "../common_defs.h"

// 日志条目结构
typedef struct LogEntry {
    char timestamp[32];
    LogLevel level;
    char message[1024];
    char source_file[256];
    int line_number;
    struct LogEntry *next;
    struct LogEntry *prev;
} LogEntry;

// 日志查看器状态
typedef struct {
    LogEntry *entries;
    LogEntry *current_entry;
    int total_entries;
    int visible_entries;
    int current_position;
    int scroll_offset;

    // 过滤选项
    LogLevel min_level;
    char filter_pattern[256];
    char source_filter[256];

    // 显示选项
    int show_timestamp;
    int show_source_location;
    int auto_refresh;
    int follow_tail;

    // 文件信息
    char log_file_path[MAX_PATH_LENGTH];
    FILE *log_file;
    long last_file_size;
} LogViewer;

// 日志查看器函数
int log_viewer_init(LogViewer *lv, const char *log_file_path);
void log_viewer_cleanup(LogViewer *lv);
int log_viewer_refresh(LogViewer *lv);
int log_viewer_reload(LogViewer *lv);

// 导航函数
LogEntry *log_viewer_get_current_entry(LogViewer *lv);
LogEntry *log_viewer_get_entry_at(LogViewer *lv, int position);
int log_viewer_scroll_up(LogViewer *lv, int lines);
int log_viewer_scroll_down(LogViewer *lv, int lines);
int log_viewer_scroll_to_top(LogViewer *lv);
int log_viewer_scroll_to_bottom(LogViewer *lv);
int log_viewer_page_up(LogViewer *lv);
int log_viewer_page_down(LogViewer *lv);

// 过滤和搜索
void log_viewer_set_level_filter(LogViewer *lv, LogLevel min_level);
void log_viewer_set_pattern_filter(LogViewer *lv, const char *pattern);
void log_viewer_set_source_filter(LogViewer *lv, const char *source);
void log_viewer_clear_filters(LogViewer *lv);
int log_viewer_search_next(LogViewer *lv, const char *pattern);
int log_viewer_search_prev(LogViewer *lv, const char *pattern);

// 显示选项
void log_viewer_toggle_timestamp(LogViewer *lv);
void log_viewer_toggle_source_location(LogViewer *lv);
void log_viewer_toggle_auto_refresh(LogViewer *lv);
void log_viewer_toggle_follow_tail(LogViewer *lv);

// 统计信息
int log_viewer_get_total_entries(LogViewer *lv);
int log_viewer_get_filtered_entries(LogViewer *lv);
int log_viewer_get_entries_by_level(LogViewer *lv, LogLevel level);

// TUI 对话框函数
void show_log_viewer_dialog(void);
void show_log_filter_dialog(LogViewer *lv);
void show_log_search_dialog(LogViewer *lv);
void show_log_statistics_dialog(LogViewer *lv);

// 日志解析函数
LogEntry *parse_log_line(const char *line);
LogLevel parse_log_level(const char *level_str);
const char *level_to_string(LogLevel level);
const char *level_to_color(LogLevel level);

#endif