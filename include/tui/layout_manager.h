#ifndef LAYOUT_MANAGER_H
#define LAYOUT_MANAGER_H

#include "../common_defs.h"

// 布局尺寸结构
typedef struct {
    int width;
    int height;
    int min_width;
    int min_height;
    int max_width;
    int max_height;
} LayoutDimensions;

// 布局位置结构
typedef struct {
    int x;
    int y;
    int alignment; // 0=左对齐, 1=居中, 2=右对齐
    int anchor;    // 锚点位置
} LayoutPosition;

// 布局边距结构
typedef struct {
    int left;
    int right;
    int top;
    int bottom;
    int horizontal; // 水平间距
    int vertical;   // 垂直间距
} LayoutMargins;

// 布局配置结构
typedef struct {
    char name[64];
    char description[256];

    // 全局布局选项
    LayoutDimensions window_size;    // 窗口大小
    LayoutMargins margins;          // 边距设置
    int spacing;                    // 元素间距

    // 对话框布局
    LayoutDimensions dialog_size;    // 对话框大小
    int dialog_border;              // 对话框边框样式
    int dialog_shadow;              // 对话框阴影

    // 菜单布局
    int menu_orientation;           // 菜单方向 (0=垂直, 1=水平)
    int menu_alignment;             // 菜单对齐方式
    int menu_spacing;               // 菜单项间距
    int menu_columns;               // 菜单列数

    // 文本布局
    int text_alignment;             // 文本对齐方式
    int text_wrap;                  // 文本自动换行
    int text_scroll;                // 文本滚动
    int line_spacing;               // 行间距

    // 颜色和视觉效果
    int enable_colors;              // 启用颜色
    int enable_bold;                // 启用粗体
    int enable_underline;           // 启用下划线
    int enable_blink;               // 启用闪烁
    int enable_reverse;             // 启用反显

    // 动画和过渡效果
    int enable_animations;          // 启用动画
    int animation_speed;            // 动画速度
    int transition_effect;          // 过渡效果

    // 响应式布局
    int responsive_design;          // 响应式设计
    int adaptive_scaling;           // 自适应缩放
    int mobile_optimized;           // 移动端优化

    // 高级选项
    int custom_css;                 // 自定义CSS
    int theme_integration;          // 主题集成
    int plugin_compatibility;       // 插件兼容性
} LayoutConfig;

// 布局管理器状态
typedef struct {
    LayoutConfig *layouts;
    LayoutConfig *current_layout;
    int layout_count;
    char layout_dir[MAX_PATH_LENGTH];

    // 运行时状态
    int screen_width;
    int screen_height;
    int terminal_type;              // 终端类型
    int color_support;              // 颜色支持级别

    // 布局缓存
    LayoutDimensions cached_dimensions;
    int cache_valid;
} LayoutManager;

// 布局管理器函数
int layout_manager_init(LayoutManager *lm, const char *layout_dir);
void layout_manager_cleanup(LayoutManager *lm);
int layout_manager_detect_terminal(LayoutManager *lm);
int layout_manager_load_layouts(LayoutManager *lm);
int layout_manager_save_layout(LayoutManager *lm, LayoutConfig *layout);

// 布局操作
int layout_manager_apply_layout(LayoutManager *lm, const char *layout_name);
int layout_manager_create_layout(LayoutManager *lm, const char *name, const char *base_layout);
int layout_manager_copy_layout(LayoutManager *lm, const char *source_name, const char *dest_name);
int layout_manager_delete_layout(LayoutManager *lm, const char *layout_name);

// 布局查询和修改
LayoutConfig *layout_manager_get_layout(LayoutManager *lm, const char *name);
int layout_manager_set_layout_property(LayoutManager *lm, const char *layout_name, const char *property, const char *value);
const char *layout_manager_get_layout_property(LayoutManager *lm, const char *layout_name, const char *property);

// 自适应布局
int layout_manager_adapt_to_screen(LayoutManager *lm, int width, int height);
int layout_manager_calculate_optimal_size(LayoutManager *lm, LayoutDimensions *content, LayoutDimensions *optimal);
int layout_manager_center_window(LayoutManager *lm, LayoutDimensions *window);

// 布局验证
int layout_manager_validate_layout(LayoutManager *lm, LayoutConfig *layout);
int layout_manager_fix_layout_issues(LayoutManager *lm, LayoutConfig *layout);

// 预设布局
int layout_manager_create_preset_layouts(LayoutManager *lm);
LayoutConfig *layout_manager_get_preset_layout(const char *name);

// TUI 对话框函数
void show_layout_manager_dialog(void);
void show_layout_selector_dialog(LayoutManager *lm);
void show_layout_editor_dialog(LayoutManager *lm, LayoutConfig *layout);
void show_layout_preview_dialog(LayoutManager *lm, LayoutConfig *layout);
void show_layout_properties_dialog(LayoutManager *lm, LayoutConfig *layout);
void show_layout_wizard_dialog(LayoutManager *lm);

// 布局应用函数
int layout_manager_apply_to_dialog(LayoutManager *lm, const char *dialog_name);
int layout_manager_apply_to_menu(LayoutManager *lm, const char *menu_name);
int layout_manager_apply_to_text(LayoutManager *lm, const char *text_element);

// 布局导入导出
int layout_manager_import_layout(LayoutManager *lm, const char *layout_file);
int layout_manager_export_layout(LayoutManager *lm, const char *layout_name, const char *export_file);

// 布局工具函数
void layout_dimensions_init(LayoutDimensions *dim, int width, int height);
void layout_margins_init(LayoutMargins *margins, int left, int right, int top, int bottom);
int layout_calculate_content_area(LayoutDimensions *window, LayoutMargins *margins, LayoutDimensions *content);
int layout_is_responsive(LayoutConfig *layout);

// 终端检测和适配
int detect_terminal_capabilities(void);
int get_optimal_dialog_size(int content_lines, int content_width);
int get_screen_dimensions(int *width, int *height);

#endif