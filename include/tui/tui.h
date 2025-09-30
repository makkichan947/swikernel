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

#endif