#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include "../common_defs.h"

// 颜色定义
typedef struct {
    char name[32];
    int color_code;
    char description[64];
} ColorDefinition;

// 主题颜色结构
typedef struct {
    // 基本颜色
    int background;        // 背景色
    int foreground;        // 前景色
    int border;           // 边框色
    int title;            // 标题色

    // 状态颜色
    int success;          // 成功色
    int warning;          // 警告色
    int error;            // 错误色
    int info;             // 信息色

    // UI元素颜色
    int menu_selected;    // 菜单选中色
    int menu_highlight;   // 菜单高亮色
    int input_background; // 输入框背景色
    int input_foreground; // 输入框前景色

    // 数据显示颜色
    int cpu_high;         // CPU高使用率
    int cpu_medium;       // CPU中等使用率
    int cpu_low;          // CPU低使用率
    int memory_high;      // 内存高使用率
    int memory_medium;    // 内存中等使用率
    int memory_low;       // 内存低使用率

    // 日志级别颜色
    int log_debug;
    int log_info;
    int log_warning;
    int log_error;
    int log_fatal;
} ThemeColors;

// 字体和布局配置
typedef struct {
    char font_family[64];     // 字体族
    int font_size;           // 字体大小
    int font_bold;           // 粗体
    int font_italic;         // 斜体

    // 布局选项
    int dialog_width;        // 对话框宽度
    int dialog_height;       // 对话框高度
    int menu_spacing;        // 菜单间距
    int border_style;        // 边框样式

    // 显示选项
    int show_icons;          // 显示图标
    int show_animations;     // 显示动画
    int show_progress_bars;  // 显示进度条
    int compact_mode;        // 紧凑模式
} ThemeLayout;

// 主题信息结构
typedef struct ThemeInfo {
    char name[64];
    char version[16];
    char author[64];
    char description[256];
    char file_path[MAX_PATH_LENGTH];

    ThemeColors colors;
    ThemeLayout layout;

    int is_builtin;          // 是否为内置主题
    int is_enabled;          // 是否启用
    time_t created_time;     // 创建时间
    time_t modified_time;    // 修改时间

    struct ThemeInfo *next;
} ThemeInfo;

// 主题管理器状态
typedef struct {
    ThemeInfo *themes;
    ThemeInfo *current_theme;
    int theme_count;
    char theme_dir[MAX_PATH_LENGTH];
    int auto_apply;          // 自动应用主题

    // 颜色缓存
    int color_enabled;
    char color_codes[256][16];
} ThemeManager;

// 主题管理器函数
int theme_manager_init(ThemeManager *tm, const char *theme_dir);
void theme_manager_cleanup(ThemeManager *tm);
int theme_manager_load_themes(ThemeManager *tm);
int theme_manager_save_theme(ThemeManager *tm, ThemeInfo *theme);
int theme_manager_delete_theme(ThemeManager *tm, const char *theme_name);

// 主题操作
int theme_manager_apply_theme(ThemeManager *tm, const char *theme_name);
int theme_manager_create_theme(ThemeManager *tm, const char *name, const char *base_theme);
int theme_manager_copy_theme(ThemeManager *tm, const char *source_name, const char *dest_name);
int theme_manager_import_theme(ThemeManager *tm, const char *theme_file);
int theme_manager_export_theme(ThemeManager *tm, const char *theme_name, const char *export_file);

// 颜色管理
int theme_manager_set_color(ThemeManager *tm, const char *theme_name, const char *color_name, int color_code);
int theme_manager_get_color(ThemeManager *tm, const char *color_name, int *color_code);
const char *theme_manager_get_color_string(ThemeManager *tm, const char *color_name);

// 内置主题
int theme_manager_create_builtin_themes(ThemeManager *tm);
ThemeInfo *theme_manager_get_builtin_theme(const char *name);

// 主题验证和修复
int theme_manager_validate_theme(ThemeManager *tm, ThemeInfo *theme);
int theme_manager_repair_theme(ThemeManager *tm, ThemeInfo *theme);

// TUI 对话框函数
void show_theme_manager_dialog(void);
void show_theme_selector_dialog(ThemeManager *tm);
void show_theme_editor_dialog(ThemeManager *tm, ThemeInfo *theme);
void show_color_picker_dialog(ThemeManager *tm, const char *color_name);
void show_theme_preview_dialog(ThemeManager *tm, ThemeInfo *theme);
void show_theme_import_dialog(ThemeManager *tm);
void show_theme_export_dialog(ThemeManager *tm);

// 主题应用函数
int theme_manager_apply_to_dialog(ThemeManager *tm);
int theme_manager_apply_to_ui(ThemeManager *tm);

// 颜色工具函数
int parse_color_code(const char *color_str);
const char *color_code_to_string(int color_code);
void get_color_palette(ColorDefinition *palette, int *count);

// 主题文件格式
int theme_manager_load_from_file(ThemeManager *tm, const char *file_path, ThemeInfo **theme);
int theme_manager_save_to_file(ThemeManager *tm, ThemeInfo *theme, const char *file_path);

#endif