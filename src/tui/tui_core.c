#include <dialog.h>
#include <string.h>
#include <stdlib.h>
#include "tui.h"
#include "logger.h"
#include "autocomplete.h"

static int dialog_initialized = 0;

// 初始化 TUI 系统
int tui_init(void) {
    if (dialog_initialized) {
        return 0;
    }
    
    // 初始化 dialog 库
    init_dialog(stdin, stdout);
    dialog_vars.backtitle = "SwiKernel - Linux Kernel Switcher";
    dialog_vars.colors = 1;
    dialog_vars.ok_label = "Select";
    dialog_vars.cancel_label = "Back";
    
    dialog_initialized = 1;
    log_message(LOG_DEBUG, "TUI system initialized");
    return 0;
}

// 清理 TUI 系统
void tui_cleanup(void) {
    if (dialog_initialized) {
        end_dialog();
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
        choice = dialog_menu("SwiKernel Main Menu",
                        "Select an operation:",
                        15, 60, 6,
                        "1", "List & Manage Kernels",
                        "2", "Install New Kernel",
                        "3", "Switch Active Kernel", 
                        "4", "Kernel Sources",
                        "5", "System Settings",
                        "6", "Exit");
        
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
                show_settings_menu();
                break;
            case 6:
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