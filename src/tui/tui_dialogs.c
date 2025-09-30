#include <dialog.h>
#include <string.h>
#include <stdlib.h>
#include "tui.h"
#include "logger.h"
#include "kernel_manager.h"

// 显示内核列表对话框
void show_kernel_list_dialog(KernelInfo *kernels) {
    if (!kernels) {
        dialog_msgbox("No Kernels", "No kernels available to display.", 8, 50);
        return;
    }
    
    int count = 0;
    KernelInfo *current = kernels;
    
    // 计算内核数量
    while (current) {
        count++;
        current = current->next;
    }
    
    // 构建对话框项
    char **items = malloc(sizeof(char*) * count * 2);
    int index = 0;
    
    current = kernels;
    while (current) {
        // 构建显示文本
        char *display = malloc(256);
        if (current->is_running) {
            snprintf(display, 256, "%-20s %-15s [RUNNING]", 
                    current->name, current->version);
        } else {
            snprintf(display, 256, "%-20s %-15s", 
                    current->name, current->version);
        }
        
        char *value = malloc(128);
        snprintf(value, 128, "%s", current->name);
        
        items[index++] = display;
        items[index++] = value;
        current = current->next;
    }
    
    char *selected = malloc(128);
    int choice = dialog_checklist("Installed Kernels", 
                                "Select kernels for more actions:",
                                18, 70, count, items, selected, 128, 0);
    
    // 清理内存
    for (int i = 0; i < count * 2; i += 2) {
        free(items[i]);
        free(items[i+1]);
    }
    free(items);
    
    if (choice == 0 && strlen(selected) > 0) {
        show_kernel_actions_dialog(selected);
    }
    
    free(selected);
}

// 安装内核对话框
void show_installation_menu(void) {
    int choice;
    
    choice = dialog_menu("Install New Kernel",
                        "Select installation method:",
                        12, 60, 4,
                        "1", "From Official Repository",
                        "2", "From Local Source Directory", 
                        "3", "From Git Repository",
                        "4", "Back");
    
    switch (choice) {
        case 1:
            install_from_repository_dialog();
            break;
        case 2:
            install_from_source_dialog();
            break;
        case 3:
            install_from_git_dialog();
            break;
        default:
            break;
    }
}

// 从源码安装对话框
void install_from_source_dialog(void) {
    char source_path[1024] = {0};
    char kernel_name[256] = {0};
    
    // 使用自动补全选择源码路径
    setup_autocomplete_dialog("Source Path", 
                            "Enter kernel source directory:",
                            AUTOCOMPLETE_PATH, 
                            source_path, sizeof(source_path));
    
    if (strlen(source_path) == 0) {
        return; // 用户取消
    }
    
    // 输入内核名称
    dialog_inputbox("Kernel Name", 
                "Enter name for this kernel:",
                10, 50, "", kernel_name, sizeof(kernel_name));
    
    if (strlen(kernel_name) == 0) {
        dialog_msgbox("Error", "Kernel name cannot be empty!", 8, 50);
        return;
    }
    
    // 确认安装
    char confirm_msg[512];
    snprintf(confirm_msg, sizeof(confirm_msg),
            "Install kernel from:\n\n"
            "Source: %s\n"
            "Name: %s\n\n"
            "This may take a long time. Continue?",
            source_path, kernel_name);
    
    if (dialog_yesno("Confirm Installation", confirm_msg, 12, 65) == 0) {
        log_message(LOG_INFO, "Starting kernel installation: %s from %s", 
                kernel_name, source_path);
        
        // 执行安装
        if (install_kernel_from_source(source_path, kernel_name) == 0) {
            dialog_msgbox("Success", "Kernel installed successfully!", 8, 50);
        } else {
            dialog_msgbox("Error", "Failed to install kernel!", 8, 50);
        }
    }
}