#include <dialog.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "tui.h"
#include "logger.h"
#include "autocomplete.h"
#include "keyboard.h"
#include "i18n.h"

static int dialog_initialized = 0;

// 键盘快捷键处理回调
void keyboard_handler(KeyBinding key, void* user_data) {
    switch (key) {
        case KEY_HELP:
            show_help_dialog();
            break;
        case KEY_REFRESH:
            // 刷新当前视图
            log_message(LOG_DEBUG, "Refresh requested");
            break;
        case KEY_SEARCH:
            show_search_dialog();
            break;
        case KEY_F1:
            // 返回主菜单 - 不需要特殊处理，主菜单始终可用
            break;
        case KEY_F2:
            show_file_manager_dialog();
            break;
        case KEY_F3:
            show_log_viewer_dialog();
            break;
        case KEY_F4:
            show_config_manager_dialog();
            break;
        case KEY_F5:
            // 刷新当前视图
            break;
        case KEY_F10:
            show_settings_menu();
            break;
        case KEY_F12:
            show_about_dialog();
            break;
        case KEY_ESCAPE:
            // 处理ESC键 - 返回上一级菜单
            break;
        default:
            log_message(LOG_DEBUG, "Unhandled key: %d", key);
            break;
    }
}

// 初始化 TUI 系统
int tui_init(void) {
    if (dialog_initialized) {
        return 0;
    }

    // 初始化 ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    // 初始化 dialog 库
    init_dialog(stdin, stdout);
    dialog_vars.backtitle = "SwiKernel - Linux Kernel Switcher";
    dialog_vars.colors = 1;
    dialog_vars.ok_label = "Select";
    dialog_vars.cancel_label = "Back";

    // 初始化键盘处理器
    if (init_keyboard_handler() != 0) {
        log_message(LOG_ERROR, "Failed to initialize keyboard handler");
        return -1;
    }

    dialog_initialized = 1;
    log_message(LOG_DEBUG, "TUI system initialized");
    return 0;
}

// 清理 TUI 系统
void tui_cleanup(void) {
    if (dialog_initialized) {
        end_dialog();

        // 清理键盘处理器
        cleanup_keyboard_handler();

        // 清理 ncurses
        endwin();

        dialog_initialized = 0;
        log_message(LOG_DEBUG, "TUI system cleaned up");
    }
}

// 主界面循环
int start_tui_interface(void) {
    if (tui_init() != 0) {
        return -1;
    }
    
    int choice;
    int ret_val = 0;
    
    while (1) {
        choice = dialog_menu(_("app_name"),
                         _("select_option"),
                         20, 70, 11,
                         "1", _("menu_kernel_management"),
                         "2", _("menu_install_kernel"),
                         "3", _("menu_switch_kernel"),
                         "4", _("menu_kernel_sources"),
                         "5", _("menu_file_manager"),
                         "6", _("menu_log_viewer"),
                         "7", _("menu_configuration"),
                         "8", _("menu_plugin_manager"),
                         "9", _("menu_system_monitor"),
                         "10", _("menu_theme_manager"),
                         "11", _("menu_layout_manager"),
                         "12", _("exit"));
        
        switch (choice) {
             case 1:
                 show_kernel_management_menu();
                 break;
             case 2:
                 show_installation_menu();
                 break;
             case 3:
                 show_switch_kernel_dialog();
                 break;
             case 4:
                 show_source_management_menu();
                 break;
             case 5:
                 show_file_manager_dialog();
                 break;
             case 6:
                 show_log_viewer_dialog();
                 break;
             case 7:
                 show_config_manager_dialog();
                 break;
             case 8:
                 show_plugin_manager_dialog();
                 break;
             case 9:
                 show_system_monitor_dashboard();
                 break;
             case 10:
                 show_theme_manager_dialog();
                 break;
             case 11:
                 show_layout_manager_dialog();
                 break;
             case 12:
             case -1: // ESC 或 Cancel
                 goto exit_loop;
             default:
                 break;
         }
    }
    
exit_loop:
    tui_cleanup();
    return ret_val;
}

// 内核管理菜单
void show_kernel_management_menu(void) {
    int choice;
    char msg[1024];
    
    // 扫描已安装的内核
    KernelInfo *kernels = scan_installed_kernels();
    KernelInfo *current = kernels;
    
    if (!kernels) {
        dialog_msgbox("No Kernels", "No installed kernels found.", 8, 50);
        return;
    }
    
    // 构建内核列表显示
    while (1) {
        // 计算统计信息
        int total = 0, running = 0;
        current = kernels;
        while (current) {
            total++;
            if (current->is_running) running++;
            current = current->next;
        }
        
        snprintf(msg, sizeof(msg), 
                "Found %d installed kernels (%d running)\n\n"
                "Select operation:",
                total, running);
        
        choice = dialog_menu("Kernel Management", msg, 16, 70, 5,
                        "1", "List All Installed Kernels",
                        "2", "Show Running Kernel",
                        "3", "Remove Kernel",
                        "4", "Kernel Information",
                        "5", "Back to Main Menu");
        
        switch (choice) {
            case 1:
                show_kernel_list_dialog(kernels);
                break;
            case 2:
                show_current_kernel_dialog();
                break;
            case 3:
                show_remove_kernel_dialog(kernels);
                break;
            case 4:
                show_kernel_info_dialog(kernels);
                break;
            case 5:
            case -1:
                goto cleanup;
        }
    }
    
cleanup:
    free_kernel_list(kernels);
}