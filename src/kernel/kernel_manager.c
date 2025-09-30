#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "kernel.h"
#include "logger.h"
#include "error_handler.h"

// 扫描已安装的内核
KernelInfo *scan_installed_kernels(void) {
    KernelInfo *head = NULL;
    KernelInfo **tail = &head;
    
    log_message(LOG_DEBUG, "Scanning for installed kernels");
    
    // 扫描 /boot 目录寻找内核镜像
    DIR *dir = opendir("/boot");
    if (!dir) {
        log_message(LOG_ERROR, "Cannot open /boot directory");
        return NULL;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 查找 vmlinuz 文件
        if (strncmp(entry->d_name, "vmlinuz-", 8) == 0) {
            KernelInfo *info = malloc(sizeof(KernelInfo));
            if (!info) {
                log_message(LOG_ERROR, "Failed to allocate kernel info");
                continue;
            }
            
            memset(info, 0, sizeof(KernelInfo));
            strcpy(info->name, entry->d_name + 8); // 跳过 "vmlinuz-"
            strcpy(info->version, info->name);
            info->installed = 1;
            
            // 检查是否是当前运行的内核
            char current_kernel[256];
            if (get_current_kernel(current_kernel, sizeof(current_kernel)) {
                info->is_running = (strcmp(info->name, current_kernel) == 0);
            }
            
            *tail = info;
            tail = &info->next;
            
            log_message(LOG_DEBUG, "Found kernel: %s (running: %d)", 
                    info->name, info->is_running);
        }
    }
    
    closedir(dir);
    return head;
}

// 获取当前运行的内核
int get_current_kernel(char *buffer, size_t size) {
    FILE *fp = fopen("/proc/version", "r");
    if (!fp) {
        log_message(LOG_ERROR, "Cannot open /proc/version");
        return 0;
    }
    
    char line[256];
    if (fgets(line, sizeof(line), fp)) {
        // 解析内核版本字符串
        char *start = strstr(line, "Linux version ");
        if (start) {
            start += 14; // 跳过 "Linux version "
            char *end = strchr(start, ' ');
            if (end) {
                size_t len = end - start;
                if (len < size) {
                    strncpy(buffer, start, len);
                    buffer[len] = '\0';
                    fclose(fp);
                    return 1;
                }
            }
        }
    }
    
    fclose(fp);
    return 0;
}

// 释放内核列表
void free_kernel_list(KernelInfo *list) {
    while (list) {
        KernelInfo *next = list->next;
        free(list);
        list = next;
    }
}

// 列出可用内核（包管理器）
int list_available_kernels(void) {
    log_message(LOG_INFO, "Listing available kernels from package manager");
    
    // 尝试不同的包管理器
    const char *package_managers[] = {"apt", "yum", "dnf", "pacman", NULL};
    
    for (int i = 0; package_managers[i]; i++) {
        if (system("which apt > /dev/null 2>&1") == 0) {
            // 使用 apt 搜索内核
            return system("apt search linux-image | grep -E '^linux-image-[0-9]' | head -20");
        }
    }
    
    for (int i = 0; package_managers[i]; i++) {
        if (system("which pacman > /dev/null 2>&1") == 0) {
            // 使用 pacman 搜索内核
            return system("pacman -Ss linux-image | grep -E '^linux-image-[0-9]' | head -20");
        }
    }

    log_message(LOG_WARNING, "No supported package manager found");
    return -1;
}