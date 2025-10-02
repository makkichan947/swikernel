#ifndef TUI_H
#define TUI_H

#include "../swikernel.h"

// TUI 初始化和管理
int tui_init(void);
void tui_cleanup(void);

// 主界面
int start_tui_interface(void);

// 对话框函数
void show_kernel_management_menu(void);
void show_kernel_list_dialog(KernelInfo *kernels);
void show_installation_menu(void);
void show_switch_kernel_dialog(void);
void show_source_management_menu(void);
void show_settings_menu(void);

// 安装相关对话框
void install_from_source_dialog(void);
void install_from_repository_dialog(void);
void install_from_git_dialog(void);
void show_kernel_actions_dialog(const char *kernel_name);
void show_current_kernel_dialog(void);
void show_remove_kernel_dialog(KernelInfo *kernels);
void show_kernel_info_dialog(KernelInfo *kernels);

// 新增快捷键支持对话框
void show_help_dialog(void);
void show_search_dialog(void);
void show_about_dialog(void);

// 文件管理对话框
void show_file_manager_dialog(void);
void show_file_info_dialog(const char *file_path);
void show_copy_dialog(FileManager *fm);
void show_move_dialog(FileManager *fm);
void show_delete_confirmation_dialog(const char *file_path);
void show_rename_dialog(const char *old_name, char *new_name, size_t size);

// 日志查看器对话框
void show_log_viewer_dialog(void);
void show_log_filter_dialog(LogViewer *lv);
void show_log_search_dialog(LogViewer *lv);
void show_log_statistics_dialog(LogViewer *lv);

// 配置管理器对话框
void show_config_manager_dialog(void);
void show_config_section_dialog(ConfigManager *cm, const char *section);
void show_config_edit_dialog(ConfigManager *cm, ConfigItem *item);
void show_config_search_dialog(ConfigManager *cm);
void show_config_validation_dialog(ConfigManager *cm);

// 插件系统对话框
void show_plugin_manager_dialog(void);
void show_plugin_info_dialog(PluginInfo *plugin);
void show_plugin_install_dialog(PluginSystem *ps);
void show_plugin_list_dialog(PluginSystem *ps);

// 系统监控仪表盘对话框
void show_system_monitor_dashboard(void);
void show_system_info_panel(SystemMonitor *sm);
void show_process_monitor_panel(SystemMonitor *sm);

// 主题管理器对话框
void show_theme_manager_dialog(void);
void show_theme_selector_dialog(ThemeManager *tm);
void show_theme_editor_dialog(ThemeManager *tm, ThemeInfo *theme);
void show_color_picker_dialog(ThemeManager *tm, const char *color_name);
void show_theme_preview_dialog(ThemeManager *tm, ThemeInfo *theme);
void show_theme_import_dialog(ThemeManager *tm);
void show_theme_export_dialog(ThemeManager *tm);

// 布局管理器对话框
void show_layout_manager_dialog(void);
void show_layout_selector_dialog(LayoutManager *lm);
void show_layout_editor_dialog(LayoutManager *lm, LayoutConfig *layout);
void show_layout_preview_dialog(LayoutManager *lm, LayoutConfig *layout);
void show_layout_properties_dialog(LayoutManager *lm, LayoutConfig *layout);

// 反馈系统对话框
void show_feedback_dialog(FeedbackSystem *fs);
void show_error_details_dialog(ErrorInfo *error);
void show_progress_dialog(ProgressInfo *progress);
void show_confirmation_dialog(const char *title, const char *message, int *result);
void show_feedback_manager_dialog(void);
void show_enhanced_error_dialog(ErrorInfo *error);
void show_operation_confirmation(const char *operation, const char *target, const char *warning);
void show_input_validation_dialog(const char *prompt, char *input, size_t size, const char *validation_pattern);
void show_status_indicator(const char *operation, int progress_percent);

#endif