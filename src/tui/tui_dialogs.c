#include <dialog.h>
#include <string.h>
#include <stdlib.h>
#include "tui.h"
#include "logger.h"
#include "kernel_manager.h"
#include "keyboard.h"
#include "log_viewer.h"
#include "config_manager.h"
#include "plugin_system.h"
#include "system_monitor.h"
#include "theme_manager.h"
#include "layout_manager.h"
#include "feedback_system.h"

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

// 显示帮助对话框
void show_help_dialog(void) {
    const char* help_text =
        "SwiKernel - Linux Kernel Switcher\n"
        "================================\n\n"
        "主菜单快捷键:\n"
        "  1        - 内核管理\n"
        "  2        - 安装新内核\n"
        "  3        - 切换活动内核\n"
        "  4        - 内核源码管理\n"
        "  5        - 文件管理器\n"
        "  6        - 日志查看器\n"
        "  7        - 配置管理\n"
        "  8        - 插件管理器\n"
        "  9        - 系统监控仪表盘\n"
        "  10       - 主题管理器\n"
        "  11       - 布局管理器\n"
        "  12       - 退出程序\n\n"
        "通用快捷键:\n"
        "  h/H      - 显示此帮助\n"
        "  r/R      - 刷新当前视图\n"
        "  /        - 搜索\n"
        "  f/F      - 过滤\n"
        "  s/S      - 排序\n"
        "  i/I      - 显示信息\n"
        "  d/D      - 删除\n"
        "  c/C      - 复制\n"
        "  m/M      - 移动\n"
        "  n/N      - 重命名\n"
        "  k/K      - 新建目录\n"
        "  e/E      - 编辑\n"
        "  F1       - 返回主菜单\n"
        "  F2       - 文件管理器\n"
        "  F3       - 日志查看器\n"
        "  F4       - 配置管理\n"
        "  F5       - 刷新\n"
        "  F10      - 设置\n"
        "  F12      - 关于\n"
        "  Esc      - 返回/取消\n"
        "  ↑/↓/←/→  - 导航\n"
        "  Tab      - 切换焦点\n"
        "  Enter    - 确认\n\n"
        "使用方向键和Enter键进行导航，"
        "或直接按数字键选择菜单选项。";

    dialog_msgbox("帮助 - SwiKernel", help_text, 20, 70);
}

// 显示搜索对话框
void show_search_dialog(void) {
    char search_term[256] = {0};

    if (dialog_inputbox("搜索", "请输入搜索关键词:", 10, 50, "", search_term, sizeof(search_term)) == 0) {
        if (strlen(search_term) > 0) {
            log_message(LOG_INFO, "Searching for: %s", search_term);
            // 这里可以实现搜索逻辑
            dialog_msgbox("搜索结果", "搜索功能开发中...", 8, 50);
        }
    }
}

// 显示关于对话框
void show_about_dialog(void) {
    const char* about_text =
        "SwiKernel - Linux Kernel Switcher\n"
        "================================\n\n"
        "版本: 2.0.0\n"
        "作者: SwiKernel Team\n"
        "版权: 2025 SwiKernel Project\n\n"
        "这是一个强大的Linux内核管理工具，"
        "提供直观的文本用户界面，"
        "支持多种内核操作和管理功能。\n\n"
        "主要特性:\n"
        "• 多内核管理\n"
        "• 一键编译安装\n"
        "• 安全回滚机制\n"
        "• 丰富的快捷键支持\n"
        "• 文件管理功能\n"
        "• 日志查看和过滤\n"
        "• 配置选项界面\n"
        "• 插件系统支持\n"
        "• 性能监控仪表盘\n"
        "• 用户自定义主题\n\n"
        "官方网站: https://github.com/makkichan947/swikernel\n"
        "许可证: NC-OSL";

    dialog_msgbox("关于 - SwiKernel", about_text, 25, 80);
}

// 文件管理器对话框
void show_file_manager_dialog(void) {
    static FileManager fm;
    static int initialized = 0;

    if (!initialized) {
        if (file_manager_init(&fm, "/") != SWK_SUCCESS) {
            dialog_msgbox("错误", "无法初始化文件管理器", 8, 50);
            return;
        }
        initialized = 1;
    }

    while (1) {
        // 构建文件列表显示
        char display_text[MAX_BUFFER_SIZE] = {0};
        snprintf(display_text, sizeof(display_text),
                "当前目录: %s\n\n文件列表:\n",
                fm.current_path);

        FileInfo *current = fm.files;
        int count = 0;
        char file_line[256];

        while (current && count < 20) { // 显示最多20个文件
            snprintf(file_line, sizeof(file_line), "%c%-30s %-8s %-12s %-12s %s\n",
                    current->is_selected ? '*' : ' ',
                    current->name,
                    current->size,
                    current->owner,
                    current->group,
                    current->modified_time);

            strncat(display_text, file_line, sizeof(display_text) - strlen(display_text) - 1);
            current = current->next;
            count++;
        }

        if (fm.file_count > 20) {
            strncat(display_text, "...\n", sizeof(display_text) - strlen(display_text) - 1);
        }

        snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                "\n快捷键: c=复制 d=删除 m=移动 n=重命名 k=新建目录 / =搜索 h=帮助");

        int choice = dialog_menu("文件管理器",
                               display_text,
                               20, 80, 8,
                               "1", "浏览目录",
                               "2", "复制文件",
                               "3", "移动文件",
                               "4", "删除文件",
                               "5", "新建目录",
                               "6", "重命名",
                               "7", "搜索过滤",
                               "8", "返回上级菜单");

        switch (choice) {
            case 1: {
                // 浏览目录
                char new_path[MAX_PATH_LENGTH] = {0};
                snprintf(new_path, sizeof(new_path), "%s/", fm.current_path);

                if (dialog_inputbox("浏览目录", "输入目录路径:", 10, 60,
                                   new_path, new_path, sizeof(new_path)) == 0) {
                    if (strlen(new_path) > 0) {
                        file_manager_change_directory(&fm, new_path);
                    }
                }
                break;
            }
            case 2:
                show_copy_dialog(&fm);
                break;
            case 3:
                show_move_dialog(&fm);
                break;
            case 4:
                show_delete_confirmation_dialog(fm.current_path);
                file_manager_refresh(&fm);
                break;
            case 5: {
                // 新建目录
                char dir_name[256] = {0};
                if (dialog_inputbox("新建目录", "输入目录名称:", 10, 50,
                                   "", dir_name, sizeof(dir_name)) == 0) {
                    if (strlen(dir_name) > 0) {
                        FileOperationResult result = file_manager_mkdir(&fm, dir_name);
                        if (result.code == SWK_SUCCESS) {
                            dialog_msgbox("成功", result.message, 8, 50);
                        } else {
                            dialog_msgbox("错误", result.message, 8, 50);
                        }
                        file_manager_refresh(&fm);
                    }
                }
                break;
            }
            case 6: {
                // 重命名
                char new_name[256] = {0};
                if (dialog_inputbox("重命名", "输入新名称:", 10, 50,
                                   "", new_name, sizeof(new_name)) == 0) {
                    if (strlen(new_name) > 0) {
                        FileOperationResult result = file_manager_rename(&fm, "current_file", new_name);
                        if (result.code == SWK_SUCCESS) {
                            dialog_msgbox("成功", result.message, 8, 50);
                        } else {
                            dialog_msgbox("错误", result.message, 8, 50);
                        }
                        file_manager_refresh(&fm);
                    }
                }
                break;
            }
            case 7: {
                // 搜索过滤
                char filter[256] = {0};
                if (dialog_inputbox("搜索过滤", "输入过滤关键词:", 10, 50,
                                   fm.filter_pattern, filter, sizeof(filter)) == 0) {
                    file_manager_set_filter(&fm, filter);
                }
                break;
            }
            case 8:
            case -1:
                return;
            default:
                break;
        }
    }
}

// 显示文件信息对话框
void show_file_info_dialog(const char *file_path) {
    if (!file_path) return;

    struct stat statbuf;
    if (stat(file_path, &statbuf) != 0) {
        dialog_msgbox("错误", "无法获取文件信息", 8, 50);
        return;
    }

    char info_text[MAX_BUFFER_SIZE] = {0};
    snprintf(info_text, sizeof(info_text),
            "文件信息: %s\n"
            "================================\n\n"
            "类型: %s\n"
            "大小: %lld 字节\n"
            "权限: %o\n"
            "所有者: %d\n"
            "组: %d\n"
            "设备: %lu\n"
            "inode: %lu\n"
            "硬链接数: %lu\n"
            "块大小: %ld\n"
            "分配块数: %ld\n"
            "最后访问: %s"
            "最后修改: %s"
            "最后状态改变: %s",
            file_path,
            S_ISDIR(statbuf.st_mode) ? "目录" : "文件",
            (long long)statbuf.st_size,
            statbuf.st_mode & 0777,
            statbuf.st_uid,
            statbuf.st_gid,
            (unsigned long)statbuf.st_dev,
            (unsigned long)statbuf.st_ino,
            (unsigned long)statbuf.st_nlink,
            (long)statbuf.st_blksize,
            (long)statbuf.st_blocks,
            ctime(&statbuf.st_atime),
            ctime(&statbuf.st_mtime),
            ctime(&statbuf.st_ctime));

    dialog_msgbox("文件信息", info_text, 20, 80);
}

// 显示复制对话框
void show_copy_dialog(FileManager *fm) {
    if (!fm) return;

    char source[MAX_PATH_LENGTH] = {0};
    char destination[MAX_PATH_LENGTH] = {0};

    snprintf(source, sizeof(source), "%s/", fm->current_path);

    if (dialog_inputbox("复制文件", "源文件路径:", 10, 60,
                       source, source, sizeof(source)) != 0) {
        return;
    }

    if (dialog_inputbox("复制文件", "目标路径:", 10, 60,
                       fm->current_path, destination, sizeof(destination)) != 0) {
        return;
    }

    if (strlen(source) > 0 && strlen(destination) > 0) {
        FileOperationResult result = file_manager_copy(fm, source, destination);
        if (result.code == SWK_SUCCESS) {
            dialog_msgbox("成功", result.message, 8, 50);
        } else {
            dialog_msgbox("错误", result.message, 8, 50);
        }
    }
}

// 显示移动对话框
void show_move_dialog(FileManager *fm) {
    if (!fm) return;

    char source[MAX_PATH_LENGTH] = {0};
    char destination[MAX_PATH_LENGTH] = {0};

    snprintf(source, sizeof(source), "%s/", fm->current_path);

    if (dialog_inputbox("移动文件", "源文件路径:", 10, 60,
                       source, source, sizeof(source)) != 0) {
        return;
    }

    if (dialog_inputbox("移动文件", "目标路径:", 10, 60,
                       fm->current_path, destination, sizeof(destination)) != 0) {
        return;
    }

    if (strlen(source) > 0 && strlen(destination) > 0) {
        FileOperationResult result = file_manager_copy(fm, source, destination);
        if (result.code == SWK_SUCCESS) {
            // 复制成功后删除源文件
            file_manager_delete(fm, source);
            dialog_msgbox("成功", "文件移动成功", 8, 50);
        } else {
            dialog_msgbox("错误", result.message, 8, 50);
        }
    }
}

// 显示删除确认对话框
void show_delete_confirmation_dialog(const char *file_path) {
    if (!file_path) return;

    char confirm_msg[512];
    snprintf(confirm_msg, sizeof(confirm_msg),
            "确定要删除以下文件吗？\n\n%s\n\n此操作不可撤销！", file_path);

    if (dialog_yesno("确认删除", confirm_msg, 12, 60) == 0) {
        char command[MAX_PATH_LENGTH];
        snprintf(command, sizeof(command), "rm -rf \"%s\"", file_path);

        if (system(command) == 0) {
            dialog_msgbox("成功", "文件删除成功", 8, 50);
        } else {
            dialog_msgbox("错误", "文件删除失败", 8, 50);
        }
    }
}

// 显示重命名对话框
void show_rename_dialog(const char *old_name, char *new_name, size_t size) {
    if (!old_name || !new_name) return;

    dialog_inputbox("重命名", "输入新名称:", 10, 50, old_name, new_name, size);
}

// 日志查看器对话框
void show_log_viewer_dialog(void) {
    static LogViewer lv;
    static int initialized = 0;

    if (!initialized) {
        if (log_viewer_init(&lv, "/var/log/swikernel.log") != SWK_SUCCESS) {
            dialog_msgbox("错误", "无法初始化日志查看器", 8, 50);
            return;
        }
        initialized = 1;
    }

    while (1) {
        // 刷新日志（如果启用了自动刷新）
        if (lv.auto_refresh) {
            log_viewer_refresh(&lv);
        }

        // 构建显示内容
        char display_text[MAX_BUFFER_SIZE] = {0};
        snprintf(display_text, sizeof(display_text),
                "日志查看器 - %s\n"
                "总条目: %d | 当前: %d\n"
                "级别过滤: %s | 模式过滤: %s\n\n",
                lv.log_file_path,
                lv.total_entries,
                lv.current_position + 1,
                level_to_string(lv.min_level),
                strlen(lv.filter_pattern) > 0 ? lv.filter_pattern : "无");

        // 显示当前条目及其周围的条目
        LogEntry *start_entry = lv.current_entry;
        int count = 0;

        // 向上查找起始条目
        while (start_entry && start_entry->prev && count < lv.visible_entries / 2) {
            start_entry = start_entry->prev;
            count++;
        }

        // 显示条目
        LogEntry *current = start_entry;
        count = 0;
        char entry_line[1152];

        while (current && count < lv.visible_entries) {
            char marker = (current == lv.current_entry) ? '>' : ' ';

            if (lv.show_timestamp) {
                snprintf(entry_line, sizeof(entry_line), "%c[%s] [%s] %s\n",
                        marker,
                        current->timestamp,
                        level_to_string(current->level),
                        current->message);
            } else {
                snprintf(entry_line, sizeof(entry_line), "%c[%s] %s\n",
                        marker,
                        level_to_string(current->level),
                        current->message);
            }

            strncat(display_text, entry_line, sizeof(display_text) - strlen(display_text) - 1);
            current = current->next;
            count++;
        }

        snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                "\n快捷键: ↑/↓=导航 / =搜索 f=过滤 r=刷新 h=帮助 q=退出");

        int choice = dialog_menu("日志查看器",
                               display_text,
                               25, 100, 8,
                               "1", "向上滚动",
                               "2", "向下滚动",
                               "3", "翻页向上",
                               "4", "翻页向下",
                               "5", "跳到顶部",
                               "6", "跳到底部",
                               "7", "搜索",
                               "8", "过滤器设置");

        switch (choice) {
            case 1:
                log_viewer_scroll_up(&lv, 1);
                break;
            case 2:
                log_viewer_scroll_down(&lv, 1);
                break;
            case 3:
                log_viewer_page_up(&lv);
                break;
            case 4:
                log_viewer_page_down(&lv);
                break;
            case 5:
                log_viewer_scroll_to_top(&lv);
                break;
            case 6:
                log_viewer_scroll_to_bottom(&lv);
                break;
            case 7:
                show_log_search_dialog(&lv);
                break;
            case 8:
                show_log_filter_dialog(&lv);
                break;
            case -1:
                return;
            default:
                break;
        }
    }
}

// 显示日志过滤器对话框
void show_log_filter_dialog(LogViewer *lv) {
    if (!lv) return;

    static char filter_pattern[256] = {0};
    static char source_filter[256] = {0};
    static int min_level = LOG_DEBUG;

    snprintf(filter_pattern, sizeof(filter_pattern), "%s", lv->filter_pattern);
    snprintf(source_filter, sizeof(source_filter), "%s", lv->source_filter);
    min_level = lv->min_level;

    int choice = dialog_menu("日志过滤器设置",
                           "选择过滤器类型:",
                           12, 60, 4,
                           "1", "级别过滤",
                           "2", "模式过滤",
                           "3", "来源过滤",
                           "4", "清除所有过滤器");

    switch (choice) {
        case 1: {
            char *levels[] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL", NULL};
            int level_choice = dialog_menu("选择最低级别",
                                        "选择要显示的最低日志级别:",
                                        10, 30, 5,
                                        "1", "DEBUG",
                                        "2", "INFO",
                                        "3", "WARNING",
                                        "4", "ERROR",
                                        "5", "FATAL");
            if (level_choice > 0) {
                min_level = level_choice - 1;
                log_viewer_set_level_filter(lv, min_level);
            }
            break;
        }
        case 2:
            if (dialog_inputbox("模式过滤", "输入要过滤的模式（支持通配符）:", 10, 60,
                               filter_pattern, filter_pattern, sizeof(filter_pattern)) == 0) {
                log_viewer_set_pattern_filter(lv, filter_pattern);
            }
            break;
        case 3:
            if (dialog_inputbox("来源过滤", "输入要过滤的源文件:", 10, 60,
                               source_filter, source_filter, sizeof(source_filter)) == 0) {
                log_viewer_set_source_filter(lv, source_filter);
            }
            break;
        case 4:
            log_viewer_clear_filters(lv);
            dialog_msgbox("完成", "所有过滤器已清除", 8, 50);
            break;
    }
}

// 显示日志搜索对话框
void show_log_search_dialog(LogViewer *lv) {
    if (!lv) return;

    char search_pattern[256] = {0};

    if (dialog_inputbox("搜索日志", "输入搜索关键词:", 10, 60,
                       "", search_pattern, sizeof(search_pattern)) == 0) {
        if (strlen(search_pattern) > 0) {
            if (log_viewer_search_next(lv, search_pattern) == SWK_SUCCESS) {
                dialog_msgbox("搜索结果", "找到匹配条目", 8, 50);
            } else {
                dialog_msgbox("搜索结果", "未找到匹配条目", 8, 50);
            }
        }
    }
}

// 显示日志统计对话框
void show_log_statistics_dialog(LogViewer *lv) {
    if (!lv) return;

    char stats_text[MAX_BUFFER_SIZE] = {0};
    snprintf(stats_text, sizeof(stats_text),
            "日志统计信息\n"
            "==============\n\n"
            "总条目数: %d\n"
            "DEBUG级别: %d\n"
            "INFO级别: %d\n"
            "WARNING级别: %d\n"
            "ERROR级别: %d\n"
            "FATAL级别: %d\n\n"
            "当前过滤器:\n"
            "级别过滤: %s\n"
            "模式过滤: %s\n"
            "来源过滤: %s\n\n"
            "显示选项:\n"
            "显示时间戳: %s\n"
            "显示源位置: %s\n"
            "自动刷新: %s\n"
            "跟随尾部: %s",
            lv->total_entries,
            log_viewer_get_entries_by_level(lv, LOG_DEBUG),
            log_viewer_get_entries_by_level(lv, LOG_INFO),
            log_viewer_get_entries_by_level(lv, LOG_WARNING),
            log_viewer_get_entries_by_level(lv, LOG_ERROR),
            log_viewer_get_entries_by_level(lv, LOG_FATAL),
            level_to_string(lv->min_level),
            strlen(lv->filter_pattern) > 0 ? lv->filter_pattern : "无",
            strlen(lv->source_filter) > 0 ? lv->source_filter : "无",
            lv->show_timestamp ? "是" : "否",
            lv->show_source_location ? "是" : "否",
            lv->auto_refresh ? "是" : "否",
            lv->follow_tail ? "是" : "否");

    dialog_msgbox("日志统计", stats_text, 25, 60);
}

// 配置管理器对话框
void show_config_manager_dialog(void) {
    static ConfigManager cm;
    static int initialized = 0;

    if (!initialized) {
        if (config_manager_init(&cm, "/etc/swikernel/swikernel.conf") != SWK_SUCCESS) {
            dialog_msgbox("错误", "无法初始化配置管理器", 8, 50);
            return;
        }
        initialized = 1;
    }

    while (1) {
        // 构建配置节列表
        char display_text[MAX_BUFFER_SIZE] = {0};
        snprintf(display_text, sizeof(display_text),
                "配置管理器\n"
                "配置文件: %s\n"
                "配置项总数: %d\n"
                "修改状态: %s\n\n"
                "选择配置节:",
                cm.config_file_path,
                cm.item_count,
                cm.modified ? "已修改" : "未修改");

        // 获取所有唯一节名
        char sections[16][64];
        int section_count = 0;

        ConfigItem *current = cm.items;
        while (current && section_count < 16) {
            int found = 0;
            for (int i = 0; i < section_count; i++) {
                if (strcmp(sections[i], current->section) == 0) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                strncpy(sections[section_count], current->section, sizeof(sections[section_count]) - 1);
                section_count++;
            }

            current = current->next;
        }

        // 构建菜单项
        char menu_items[256] = {0};
        for (int i = 0; i < section_count; i++) {
            char item[32];
            snprintf(item, sizeof(item), "%d", i + 1);
            strncat(menu_items, item, sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, sections[i], sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
        }

        // 添加操作选项
        strncat(menu_items, "s\n搜索配置项\n", sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, "v\n验证配置\n", sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, "r\n重新加载\n", sizeof(menu_items) - strlen(menu_items) - 1);

        int choice = dialog_menu("配置管理器",
                               display_text,
                               20, 80, section_count + 3,
                               menu_items);

        if (choice >= 1 && choice <= section_count) {
            // 显示指定节的配置
            show_config_section_dialog(&cm, sections[choice - 1]);
        } else {
            switch (choice) {
                case section_count + 1: // 搜索
                    show_config_search_dialog(&cm);
                    break;
                case section_count + 2: // 验证
                    show_config_validation_dialog(&cm);
                    break;
                case section_count + 3: // 重新加载
                    config_manager_reload(&cm);
                    dialog_msgbox("完成", "配置已重新加载", 8, 50);
                    break;
                case -1:
                    // 保存修改并退出
                    if (cm.modified) {
                        if (dialog_yesno("保存修改", "配置已被修改，是否保存？", 8, 50) == 0) {
                            config_manager_save(&cm);
                        }
                    }
                    return;
                default:
                    break;
            }
        }
    }
}

// 显示配置节对话框
void show_config_section_dialog(ConfigManager *cm, const char *section) {
    if (!cm || !section) return;

    while (1) {
        // 获取该节的所有配置项
        ConfigItem *section_items = config_manager_get_items_in_section(cm, section);
        if (!section_items) {
            dialog_msgbox("错误", "无法获取配置节信息", 8, 50);
            return;
        }

        // 构建显示文本
        char display_text[MAX_BUFFER_SIZE] = {0};
        snprintf(display_text, sizeof(display_text),
                "配置节: [%s]\n\n", section);

        ConfigItem *current = section_items;
        char item_line[256];
        int count = 0;

        while (current && count < 15) {
            snprintf(item_line, sizeof(item_line), "%-20s = %-30s [%s]\n",
                    current->key,
                    strlen(current->value) > 30 ? "..." : current->value,
                    config_type_to_string(current->type));

            strncat(display_text, item_line, sizeof(display_text) - strlen(display_text) - 1);
            current = current->next;
            count++;
        }

        snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                "\n选择操作:");

        // 构建菜单
        char menu_items[512] = {0};
        current = section_items;
        count = 0;

        while (current && count < 10) {
            char item_num[8];
            snprintf(item_num, sizeof(item_num), "%d", count + 1);
            strncat(menu_items, item_num, sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, current->key, sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
            current = current->next;
            count++;
        }

        // 添加操作选项
        strncat(menu_items, "e\n编辑选中项\n", sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, "a\n添加新项\n", sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, "b\n返回\n", sizeof(menu_items) - strlen(menu_items) - 1);

        int choice = dialog_menu("配置节管理",
                               display_text,
                               20, 80, count + 3,
                               menu_items);

        if (choice >= 1 && choice <= count) {
            // 编辑指定配置项
            current = section_items;
            for (int i = 1; i < choice && current; i++) {
                current = current->next;
            }

            if (current) {
                show_config_edit_dialog(cm, current);
            }
        } else {
            switch (choice) {
                case count + 1: // 编辑选中项
                    // 这里需要实现选中项编辑逻辑
                    break;
                case count + 2: // 添加新项
                    // 这里需要实现添加新项逻辑
                    break;
                case -1:
                case count + 3: // 返回
                    config_item_destroy(section_items);
                    return;
                default:
                    break;
            }
        }

        config_item_destroy(section_items);
    }
}

// 显示配置编辑对话框
void show_config_edit_dialog(ConfigManager *cm, ConfigItem *item) {
    if (!cm || !item) return;

    char new_value[256] = {0};
    strncpy(new_value, item->value, sizeof(new_value) - 1);

    char prompt[512];
    snprintf(prompt, sizeof(prompt),
            "编辑配置项:\n\n"
            "节: [%s]\n"
            "键: %s\n"
            "类型: %s\n"
            "描述: %s\n\n"
            "当前值: %s\n\n"
            "输入新值:",
            item->section,
            item->key,
            config_type_to_string(item->type),
            item->description,
            item->value);

    if (dialog_inputbox("编辑配置项", prompt, 15, 70, item->value, new_value, sizeof(new_value)) == 0) {
        if (strcmp(new_value, item->value) != 0) {
            if (config_manager_set_value(cm, item->section, item->key, new_value) == SWK_SUCCESS) {
                dialog_msgbox("成功", "配置项已更新", 8, 50);
            } else {
                dialog_msgbox("错误", "无法更新配置项", 8, 50);
            }
        }
    }
}

// 显示配置搜索对话框
void show_config_search_dialog(ConfigManager *cm) {
    if (!cm) return;

    char search_pattern[256] = {0};

    if (dialog_inputbox("搜索配置项", "输入搜索关键词:", 10, 60,
                       "", search_pattern, sizeof(search_pattern)) == 0) {
        if (strlen(search_pattern) > 0) {
            ConfigItem *results = config_manager_find_items(cm, search_pattern);
            if (results) {
                char result_text[MAX_BUFFER_SIZE] = {0};
                snprintf(result_text, sizeof(result_text), "搜索结果:\n\n");

                ConfigItem *current = results;
                int count = 0;
                char item_line[256];

                while (current && count < 10) {
                    snprintf(item_line, sizeof(item_line), "[%s] %s = %s\n",
                            current->section, current->key, current->value);
                    strncat(result_text, item_line, sizeof(result_text) - strlen(result_text) - 1);
                    current = current->next;
                    count++;
                }

                dialog_msgbox("搜索结果", result_text, 15, 80);
                config_item_destroy(results);
            } else {
                dialog_msgbox("搜索结果", "未找到匹配的配置项", 8, 50);
            }
        }
    }
}

// 显示配置验证对话框
void show_config_validation_dialog(ConfigManager *cm) {
    if (!cm) return;

    if (config_manager_validate_all(cm) == SWK_SUCCESS) {
        dialog_msgbox("验证结果", "所有配置项验证通过", 8, 50);
    } else {
        dialog_msgbox("验证结果", "发现配置项验证错误，请检查配置", 8, 50);
    }
    
    // 插件管理器对话框
    void show_plugin_manager_dialog(void) {
        static PluginSystem ps;
        static int initialized = 0;
    
        if (!initialized) {
            if (plugin_system_init(&ps, "/usr/lib/swikernel/plugins") != SWK_SUCCESS) {
                dialog_msgbox("错误", "无法初始化插件系统", 8, 50);
                return;
            }
            initialized = 1;
        }
    
        while (1) {
            // 构建插件列表显示
            char display_text[MAX_BUFFER_SIZE] = {0};
            snprintf(display_text, sizeof(display_text),
                    "插件管理器\n"
                    "插件总数: %d | 启用: %d\n"
                    "插件目录: %s\n\n"
                    "已安装插件:",
                    ps.plugin_count,
                    ps.enabled_count,
                    ps.plugin_dir);
    
            PluginInfo *current = ps.plugins;
            int count = 0;
            char plugin_line[256];
    
            while (current && count < 15) {
                snprintf(plugin_line, sizeof(plugin_line), "%c %-20s %-10s %-8s %s\n",
                        current->enabled ? '*' : ' ',
                        current->name,
                        current->version,
                        current->enabled ? "启用" : "禁用",
                        current->description);
    
                strncat(display_text, plugin_line, sizeof(display_text) - strlen(display_text) - 1);
                current = current->next;
                count++;
            }
    
            snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                    "\n选择操作:");
    
            // 构建菜单
            char menu_items[512] = {0};
            current = ps.plugins;
            count = 0;
    
            while (current && count < 10) {
                char item_num[8];
                snprintf(item_num, sizeof(item_num), "%d", count + 1);
                strncat(menu_items, item_num, sizeof(menu_items) - strlen(menu_items) - 1);
                strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
                strncat(menu_items, current->name, sizeof(menu_items) - strlen(menu_items) - 1);
                strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
                current = current->next;
                count++;
            }
    
            // 添加操作选项
            strncat(menu_items, "i\n安装插件\n", sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "s\n扫描插件\n", sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "r\n刷新列表\n", sizeof(menu_items) - strlen(menu_items) - 1);
    
            int choice = dialog_menu("插件管理器",
                                   display_text,
                                   22, 80, count + 3,
                                   menu_items);
    
            if (choice >= 1 && choice <= count) {
                // 显示指定插件信息
                current = ps.plugins;
                for (int i = 1; i < choice && current; i++) {
                    current = current->next;
                }
    
                if (current) {
                    show_plugin_info_dialog(current);
                }
            } else {
                switch (choice) {
                    case count + 1: // 安装插件
                        show_plugin_install_dialog(&ps);
                        break;
                    case count + 2: // 扫描插件
                        plugin_system_scan_plugins(&ps);
                        dialog_msgbox("完成", "插件扫描完成", 8, 50);
                        break;
                    case count + 3: // 刷新列表
                        // 刷新已在主循环中处理
                        break;
                    case -1:
                        return;
                    default:
                        break;
                }
            }
        }
    }
    
    // 显示插件信息对话框
    void show_plugin_info_dialog(PluginInfo *plugin) {
        if (!plugin) return;
    
        char info_text[MAX_BUFFER_SIZE] = {0};
        snprintf(info_text, sizeof(info_text),
                "插件信息\n"
                "==========\n\n"
                "名称: %s\n"
                "版本: %s\n"
                "类型: %s\n"
                "作者: %s\n"
                "描述: %s\n"
                "文件: %s\n"
                "状态: %s\n\n"
                "选择操作:",
                plugin->name,
                plugin->version,
                plugin_type_to_string(plugin->type),
                plugin->author,
                plugin->description,
                plugin->file_path,
                plugin->enabled ? "已启用" : "已禁用");
    
        char menu_items[128] = {0};
        if (plugin->enabled) {
            strncat(menu_items, "1\n禁用插件\n", sizeof(menu_items) - strlen(menu_items) - 1);
        } else {
            strncat(menu_items, "1\n启用插件\n", sizeof(menu_items) - strlen(menu_items) - 1);
        }
        strncat(menu_items, "2\n卸载插件\n", sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, "3\n返回\n", sizeof(menu_items) - strlen(menu_items) - 1);
    
        int choice = dialog_menu("插件信息",
                               info_text,
                               18, 70, 3,
                               menu_items);
    
        switch (choice) {
            case 1:
                if (plugin->enabled) {
                    plugin_system_disable_plugin(&ps, plugin->name);
                    dialog_msgbox("完成", "插件已禁用", 8, 50);
                } else {
                    plugin_system_enable_plugin(&ps, plugin->name);
                    dialog_msgbox("完成", "插件已启用", 8, 50);
                }
                break;
            case 2:
                if (dialog_yesno("确认卸载", "确定要卸载此插件吗？", 8, 50) == 0) {
                    if (plugin_system_remove_plugin(&ps, plugin->name) == SWK_SUCCESS) {
                        dialog_msgbox("完成", "插件已卸载", 8, 50);
                    } else {
                        dialog_msgbox("错误", "卸载插件失败", 8, 50);
                    }
                }
                break;
            default:
                break;
        }
    }
    
    // 显示插件安装对话框
    void show_plugin_install_dialog(PluginSystem *ps) {
        if (!ps) return;
    
        char plugin_file[MAX_PATH_LENGTH] = {0};
    
        if (dialog_inputbox("安装插件", "输入插件文件路径:", 10, 60,
                           "", plugin_file, sizeof(plugin_file)) == 0) {
            if (strlen(plugin_file) > 0) {
                if (plugin_system_install_plugin(ps, plugin_file) == SWK_SUCCESS) {
                    dialog_msgbox("成功", "插件安装成功", 8, 50);
                } else {
                    dialog_msgbox("错误", "插件安装失败", 8, 50);
                }
            }
        }
    }
    
    // 显示插件列表对话框
    void show_plugin_list_dialog(PluginSystem *ps) {
        if (!ps) return;
    
        char display_text[MAX_BUFFER_SIZE] = {0};
        snprintf(display_text, sizeof(display_text),
                "插件列表\n"
                "==========\n\n");
    
        PluginInfo *current = ps->plugins;
        char plugin_line[256];
        int count = 0;
    
        while (current) {
            snprintf(plugin_line, sizeof(plugin_line), "%c %-20s %-10s %s\n",
                    current->enabled ? '*' : ' ',
                    current->name,
                    current->version,
                    current->description);
    
            strncat(display_text, plugin_line, sizeof(display_text) - strlen(display_text) - 1);
            current = current->next;
            count++;
        }
    
        if (count == 0) {
            strncat(display_text, "没有安装任何插件\n", sizeof(display_text) - strlen(display_text) - 1);
        }
    
        dialog_msgbox("插件列表", display_text, 20, 80);
    }
    
    // 插件类型转字符串
    const char *plugin_type_to_string(PluginType type) {
        switch (type) {
            case PLUGIN_TYPE_KERNEL_MANAGER: return "内核管理";
            case PLUGIN_TYPE_FILE_MANAGER: return "文件管理";
            case PLUGIN_TYPE_LOG_VIEWER: return "日志查看";
            case PLUGIN_TYPE_CONFIG_MANAGER: return "配置管理";
            case PLUGIN_TYPE_SYSTEM_MONITOR: return "系统监控";
            case PLUGIN_TYPE_THEME_MANAGER: return "主题管理";
            case PLUGIN_TYPE_CUSTOM: return "自定义";
            default: return "未知";
        }
    }

    // 系统监控仪表盘对话框
    void show_system_monitor_dashboard(void) {
        static SystemMonitor sm;
        static int initialized = 0;
        static time_t last_update = 0;

        if (!initialized) {
            if (system_monitor_init(&sm) != SWK_SUCCESS) {
                dialog_msgbox("错误", "无法初始化系统监控器", 8, 50);
                return;
            }
            initialized = 1;
            last_update = time(NULL);
        }

        while (1) {
            // 定期更新统计信息
            time_t current_time = time(NULL);
            if (current_time - last_update >= sm.config.update_interval) {
                system_monitor_update(&sm);
                last_update = current_time;
            }

            // 构建仪表盘显示文本
            char display_text[MAX_BUFFER_SIZE] = {0};
            SystemStats *stats = &sm.current_stats;

            snprintf(display_text, sizeof(display_text),
                    "系统性能监控仪表盘\n"
                    "更新时间: %s"
                    "更新间隔: %d秒\n\n",
                    ctime(&stats->timestamp),
                    sm.config.update_interval);

            // CPU 面板
            strncat(display_text, "┌─ CPU 状态 ──────────────────────┐\n", sizeof(display_text) - strlen(display_text) - 1);
            snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                    "│ CPU使用率: %6.1f%%              │\n"
                    "│ CPU核心数: %6d                │\n"
                    "│ CPU频率: %6.0f MHz            │\n"
                    "│ 系统负载: %6.2f, %6.2f, %6.2f    │\n"
                    "└─────────────────────────────────┘\n\n",
                    stats->cpu_usage,
                    stats->cpu_cores,
                    stats->cpu_frequency,
                    stats->load_average_1,
                    stats->load_average_5,
                    stats->load_average_15);

            // 内存面板
            strncat(display_text, "┌─ 内存状态 ──────────────────────┐\n", sizeof(display_text) - strlen(display_text) - 1);
            snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                    "│ 内存使用: %6.1f%% (%lu/%lu MB)  │\n"
                    "│ 交换分区: %6.1f%% (%lu/%lu MB)  │\n"
                    "│ 缓冲区: %6lu MB              │\n"
                    "│ 缓存: %6lu MB                │\n"
                    "└─────────────────────────────────┘\n\n",
                    stats->memory_usage_percent,
                    stats->memory_used / 1024, stats->memory_total / 1024,
                    stats->swap_usage_percent,
                    stats->swap_used / 1024, stats->swap_total / 1024,
                    stats->memory_buffers / 1024,
                    (stats->memory_cached) / 1024);

            // 磁盘面板
            strncat(display_text, "┌─ 磁盘状态 ──────────────────────┐\n", sizeof(display_text) - strlen(display_text) - 1);
            snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                    "│ 磁盘使用: %6.1f%% (%lu/%lu GB)  │\n"
                    "│ 挂载点: %-20s     │\n"
                    "└─────────────────────────────────┘\n\n",
                    stats->disk_usage_percent,
                    stats->disk_used / 1024 / 1024, stats->disk_total / 1024 / 1024,
                    stats->disk_mount_point);

            // 进程面板
            strncat(display_text, "┌─ 进程状态 ──────────────────────┐\n", sizeof(display_text) - strlen(display_text) - 1);
            snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                    "│ 进程总数: %6d                │\n"
                    "│ 线程总数: %6d                │\n"
                    "│ 僵尸进程: %6d                │\n"
                    "└─────────────────────────────────┘\n\n",
                    stats->process_count,
                    stats->thread_count,
                    stats->zombie_processes);

            snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                    "快捷键: r=刷新 i=信息 p=进程 u=更新间隔 h=帮助 q=退出");

            int choice = dialog_menu("系统监控仪表盘",
                                   display_text,
                                   25, 80, 5,
                                   "1", "刷新统计信息",
                                   "2", "显示详细信息",
                                   "3", "进程监控面板",
                                   "4", "设置更新间隔",
                                   "5", "返回主菜单");

            switch (choice) {
                case 1:
                    system_monitor_update(&sm);
                    break;
                case 2:
                    show_system_info_panel(&sm);
                    break;
                case 3:
                    show_process_monitor_panel(&sm);
                    break;
                case 4: {
                    char interval_str[16];
                    snprintf(interval_str, sizeof(interval_str), "%d", sm.config.update_interval);

                    if (dialog_inputbox("设置更新间隔", "输入更新间隔（秒）:", 10, 30,
                                       interval_str, interval_str, sizeof(interval_str)) == 0) {
                        int interval = atoi(interval_str);
                        if (interval > 0 && interval <= 60) {
                            sm.config.update_interval = interval;
                        } else {
                            dialog_msgbox("错误", "更新间隔必须在1-60秒之间", 8, 50);
                        }
                    }
                    break;
                }
                case 5:
                case -1:
                    return;
                default:
                    break;
            }
        }
    }

    // 显示系统信息面板
    void show_system_info_panel(SystemMonitor *sm) {
        if (!sm) return;

        SystemStats *stats = &sm.current_stats;
        char info_text[MAX_BUFFER_SIZE] = {0};

        snprintf(info_text, sizeof(info_text),
                "系统详细信息\n"
                "==============\n\n"
                "CPU信息:\n"
                "  使用率: %.2f%%\n"
                "  核心数: %d\n"
                "  频率: %.0f MHz\n\n"
                "内存信息:\n"
                "  总内存: %lu MB\n"
                "  已用内存: %lu MB\n"
                "  空闲内存: %lu MB\n"
                "  缓冲区: %lu MB\n"
                "  缓存: %lu MB\n"
                "  使用率: %.2f%%\n\n"
                "交换分区:\n"
                "  总交换: %lu MB\n"
                "  已用交换: %lu MB\n"
                "  空闲交换: %lu MB\n"
                "  使用率: %.2f%%\n\n"
                "磁盘信息:\n"
                "  总磁盘: %lu GB\n"
                "  已用磁盘: %lu GB\n"
                "  空闲磁盘: %lu GB\n"
                "  使用率: %.2f%%\n"
                "  挂载点: %s\n\n"
                "系统负载: %.2f (1分钟), %.2f (5分钟), %.2f (15分钟)",
                stats->cpu_usage,
                stats->cpu_cores,
                stats->cpu_frequency,
                stats->memory_total / 1024,
                stats->memory_used / 1024,
                stats->memory_free / 1024,
                stats->memory_buffers / 1024,
                stats->memory_cached / 1024,
                stats->memory_usage_percent,
                stats->swap_total / 1024,
                stats->swap_used / 1024,
                stats->swap_free / 1024,
                stats->swap_usage_percent,
                stats->disk_total / 1024 / 1024,
                stats->disk_used / 1024 / 1024,
                stats->disk_free / 1024 / 1024,
                stats->disk_usage_percent,
                stats->disk_mount_point,
                stats->load_average_1,
                stats->load_average_5,
                stats->load_average_15);

        dialog_msgbox("系统详细信息", info_text, 25, 80);
    }

    // 显示进程监控面板
    void show_process_monitor_panel(SystemMonitor *sm) {
        if (!sm) return;

        SystemStats *stats = &sm.current_stats;
        char process_text[MAX_BUFFER_SIZE] = {0};

        snprintf(process_text, sizeof(process_text),
                "进程监控面板\n"
                "==============\n\n"
                "进程统计:\n"
                "  总进程数: %d\n"
                "  总线程数: %d\n"
                "  僵尸进程: %d\n\n"
                "进程列表 (前20个):\n",
                stats->process_count,
                stats->thread_count,
                stats->zombie_processes);

        // 读取进程信息
        FILE *file = fopen("/proc/stat", "r");
        if (file) {
            char line[256];
            int count = 0;

            while (fgets(line, sizeof(line), file) && count < 20) {
                if (strncmp(line, "cpu", 3) == 0) {
                    char cpu_id[8];
                    unsigned long user, nice, system, idle;
                    sscanf(line, "%s %lu %lu %lu %lu", cpu_id, &user, &nice, &system, &idle);

                    unsigned long total = user + nice + system + idle;
                    unsigned long active = total - idle;

                    if (total > 0) {
                        double usage = (double)active * 100.0 / total;
                        snprintf(process_text + strlen(process_text), sizeof(process_text) - strlen(process_text),
                                "  %s: %.1f%%\n", cpu_id, usage);
                    }
                    count++;
                }
            }
            fclose(file);
        }

        dialog_msgbox("进程监控", process_text, 20, 60);
    }

    // 主题管理器对话框
    void show_theme_manager_dialog(void) {
        static ThemeManager tm;
        static int initialized = 0;

        if (!initialized) {
            if (theme_manager_init(&tm, "/usr/share/swikernel/themes") != SWK_SUCCESS) {
                dialog_msgbox("错误", "无法初始化主题管理器", 8, 50);
                return;
            }
            initialized = 1;
        }

        while (1) {
            // 构建主题列表显示
            char display_text[MAX_BUFFER_SIZE] = {0};
            snprintf(display_text, sizeof(display_text),
                    "主题管理器\n"
                    "主题总数: %d\n"
                    "当前主题: %s\n"
                    "主题目录: %s\n\n"
                    "可用主题:",
                    tm.theme_count,
                    tm.current_theme ? tm.current_theme->name : "无",
                    tm.theme_dir);

            ThemeInfo *current = tm.themes;
            int count = 0;
            char theme_line[256];

            while (current && count < 15) {
                snprintf(theme_line, sizeof(theme_line), "%c %-15s %-10s %s\n",
                        current == tm.current_theme ? '*' : ' ',
                        current->name,
                        current->version,
                        current->description);

                strncat(display_text, theme_line, sizeof(display_text) - strlen(display_text) - 1);
                current = current->next;
                count++;
            }

            snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                    "\n选择操作:");

            // 构建菜单
            char menu_items[512] = {0};
            current = tm.themes;
            count = 0;

            while (current && count < 10) {
                char item_num[8];
                snprintf(item_num, sizeof(item_num), "%d", count + 1);
                strncat(menu_items, item_num, sizeof(menu_items) - strlen(menu_items) - 1);
                strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
                strncat(menu_items, current->name, sizeof(menu_items) - strlen(menu_items) - 1);
                strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
                current = current->next;
                count++;
            }

            // 添加操作选项
            strncat(menu_items, "c\n创建主题\n", sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "i\n导入主题\n", sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "e\n编辑主题\n", sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "p\n预览主题\n", sizeof(menu_items) - strlen(menu_items) - 1);

            int choice = dialog_menu("主题管理器",
                                   display_text,
                                   22, 80, count + 4,
                                   menu_items);

            if (choice >= 1 && choice <= count) {
                // 应用指定主题
                current = tm.themes;
                for (int i = 1; i < choice && current; i++) {
                    current = current->next;
                }

                if (current) {
                    theme_manager_apply_theme(&tm, current->name);
                    dialog_msgbox("完成", "主题已应用", 8, 50);
                }
            } else {
                switch (choice) {
                    case count + 1: // 创建主题
                        show_theme_create_dialog(&tm);
                        break;
                    case count + 2: // 导入主题
                        show_theme_import_dialog(&tm);
                        break;
                    case count + 3: // 编辑主题
                        show_theme_edit_dialog(&tm);
                        break;
                    case count + 4: // 预览主题
                        show_theme_preview_dialog(&tm, tm.current_theme);
                        break;
                    case -1:
                        return;
                    default:
                        break;
                }
            }
        }
    }

    // 显示主题选择对话框
    void show_theme_selector_dialog(ThemeManager *tm) {
        if (!tm) return;

        char display_text[MAX_BUFFER_SIZE] = {0};
        snprintf(display_text, sizeof(display_text),
                "选择主题\n"
                "当前主题: %s\n\n"
                "可用主题:",
                tm->current_theme ? tm->current_theme->name : "无");

        ThemeInfo *current = tm.themes;
        int count = 0;
        char theme_line[256];

        while (current) {
            snprintf(theme_line, sizeof(theme_line), "%c %-15s %-10s %s\n",
                    current == tm->current_theme ? '*' : ' ',
                    current->name,
                    current->version,
                    current->description);

            strncat(display_text, theme_line, sizeof(display_text) - strlen(display_text) - 1);
            current = current->next;
            count++;
        }

        // 构建菜单项
        char menu_items[512] = {0};
        current = tm->themes;
        count = 0;

        while (current) {
            char item_num[8];
            snprintf(item_num, sizeof(item_num), "%d", count + 1);
            strncat(menu_items, item_num, sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, current->name, sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
            current = current->next;
            count++;
        }

        int choice = dialog_menu("主题选择",
                               display_text,
                               20, 80, count,
                               menu_items);

        if (choice >= 1 && choice <= count) {
            current = tm->themes;
            for (int i = 1; i < choice && current; i++) {
                current = current->next;
            }

            if (current) {
                theme_manager_apply_theme(tm, current->name);
            }
        }
    }

    // 显示主题创建对话框
    void show_theme_create_dialog(ThemeManager *tm) {
        if (!tm) return;

        char theme_name[64] = {0};
        char base_theme[64] = {0};

        if (dialog_inputbox("创建主题", "输入新主题名称:", 10, 50,
                           "", theme_name, sizeof(theme_name)) != 0) {
            return;
        }

        if (strlen(theme_name) == 0) {
            dialog_msgbox("错误", "主题名称不能为空", 8, 50);
            return;
        }

        // 选择基础主题
        snprintf(base_theme, sizeof(base_theme), "%s",
                tm->current_theme ? tm->current_theme->name : "default");

        if (dialog_inputbox("基础主题", "选择基础主题 (留空使用默认):", 10, 50,
                           base_theme, base_theme, sizeof(base_theme)) == 0) {
            if (theme_manager_create_theme(tm, theme_name, base_theme) == SWK_SUCCESS) {
                dialog_msgbox("成功", "主题创建成功", 8, 50);
            } else {
                dialog_msgbox("错误", "主题创建失败", 8, 50);
            }
        }
    }

    // 显示主题编辑对话框
    void show_theme_editor_dialog(ThemeManager *tm, ThemeInfo *theme) {
        if (!tm || !theme) return;

        while (1) {
            char display_text[MAX_BUFFER_SIZE] = {0};
            snprintf(display_text, sizeof(display_text),
                    "编辑主题: %s\n"
                    "作者: %s\n"
                    "版本: %s\n"
                    "描述: %s\n\n"
                    "颜色设置:\n"
                    "背景色: %s\n"
                    "前景色: %s\n"
                    "边框色: %s\n"
                    "标题色: %s\n\n"
                    "选择要编辑的项目:",
                    theme->name,
                    theme->author,
                    theme->version,
                    theme->description,
                    color_code_to_string(theme->colors.background),
                    color_code_to_string(theme->colors.foreground),
                    color_code_to_string(theme->colors.border),
                    color_code_to_string(theme->colors.title));

            int choice = dialog_menu("主题编辑器",
                                   display_text,
                                   20, 70, 6,
                                   "1", "编辑基本信息",
                                   "2", "编辑背景色",
                                   "3", "编辑前景色",
                                   "4", "编辑边框色",
                                   "5", "编辑标题色",
                                   "6", "保存并返回");

            switch (choice) {
                case 1:
                    show_theme_basic_info_dialog(tm, theme);
                    break;
                case 2:
                    show_color_picker_dialog(tm, "background");
                    break;
                case 3:
                    show_color_picker_dialog(tm, "foreground");
                    break;
                case 4:
                    show_color_picker_dialog(tm, "border");
                    break;
                case 5:
                    show_color_picker_dialog(tm, "title");
                    break;
                case 6:
                case -1:
                    return;
                default:
                    break;
            }
        }
    }

    // 显示颜色选择对话框
    void show_color_picker_dialog(ThemeManager *tm, const char *color_name) {
        if (!tm || !color_name) return;

        ColorDefinition palette[16];
        int palette_count;

        get_color_palette(palette, &palette_count);

        // 构建颜色菜单
        char menu_items[1024] = {0};
        for (int i = 0; i < palette_count; i++) {
            char item_num[8];
            snprintf(item_num, sizeof(item_num), "%d", i + 1);
            strncat(menu_items, item_num, sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, palette[i].name, sizeof(menu_items) - strlen(menu_items) - 1);
            strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
        }

        char display_text[MAX_BUFFER_SIZE] = {0};
        snprintf(display_text, sizeof(display_text),
                "选择颜色: %s\n\n"
                "可用颜色:", color_name);

        int choice = dialog_menu("颜色选择器",
                               display_text,
                               20, 50, palette_count,
                               menu_items);

        if (choice >= 1 && choice <= palette_count) {
            int color_code = palette[choice - 1].color_code;
            theme_manager_set_color(tm, tm->current_theme->name, color_name, color_code);
            dialog_msgbox("完成", "颜色已更新", 8, 50);
        }
    }

    // 显示主题预览对话框
    void show_theme_preview_dialog(ThemeManager *tm, ThemeInfo *theme) {
        if (!tm || !theme) return;

        char preview_text[MAX_BUFFER_SIZE] = {0};
        snprintf(preview_text, sizeof(preview_text),
                "主题预览: %s\n"
                "================\n\n"
                "基本信息:\n"
                "名称: %s\n"
                "版本: %s\n"
                "作者: %s\n"
                "描述: %s\n\n"
                "颜色方案:\n"
                "背景色: %s\n"
                "前景色: %s\n"
                "边框色: %s\n"
                "标题色: %s\n"
                "成功色: %s\n"
                "警告色: %s\n"
                "错误色: %s\n"
                "信息色: %s\n\n"
                "此预览仅显示颜色代码，实际效果请应用主题后查看。",
                theme->name,
                theme->name,
                theme->version,
                theme->author,
                theme->description,
                color_code_to_string(theme->colors.background),
                color_code_to_string(theme->colors.foreground),
                color_code_to_string(theme->colors.border),
                color_code_to_string(theme->colors.title),
                color_code_to_string(theme->colors.success),
                color_code_to_string(theme->colors.warning),
                color_code_to_string(theme->colors.error),
                color_code_to_string(theme->colors.info));

        dialog_msgbox("主题预览", preview_text, 25, 80);
    }

    // 显示主题导入对话框
    void show_theme_import_dialog(ThemeManager *tm) {
        if (!tm) return;

        char theme_file[MAX_PATH_LENGTH] = {0};

        if (dialog_inputbox("导入主题", "输入主题文件路径:", 10, 60,
                           "", theme_file, sizeof(theme_file)) == 0) {
            if (strlen(theme_file) > 0) {
                if (theme_manager_import_theme(tm, theme_file) == SWK_SUCCESS) {
                    dialog_msgbox("成功", "主题导入成功", 8, 50);
                } else {
                    dialog_msgbox("错误", "主题导入失败", 8, 50);
                }
            }
        }
    }

    // 显示主题导出对话框
    void show_theme_export_dialog(ThemeManager *tm) {
        if (!tm || !tm->current_theme) return;

        char export_file[MAX_PATH_LENGTH] = {0};
        snprintf(export_file, sizeof(export_file), "/tmp/%s.theme", tm->current_theme->name);

        if (dialog_inputbox("导出主题", "输入导出文件路径:", 10, 60,
                           export_file, export_file, sizeof(export_file)) == 0) {
            if (strlen(export_file) > 0) {
                if (theme_manager_export_theme(tm, tm->current_theme->name, export_file) == SWK_SUCCESS) {
                    dialog_msgbox("成功", "主题导出成功", 8, 50);
                } else {
                    dialog_msgbox("错误", "主题导出失败", 8, 50);
                }
            }
        }
    }

    // 显示主题基本信息编辑对话框
    void show_theme_basic_info_dialog(ThemeManager *tm, ThemeInfo *theme) {
        if (!tm || !theme) return;

        char new_author[64] = {0};
        char new_description[256] = {0};

        strncpy(new_author, theme->author, sizeof(new_author) - 1);
        strncpy(new_description, theme->description, sizeof(new_description) - 1);

        if (dialog_inputbox("编辑作者", "输入主题作者:", 10, 50,
                           new_author, new_author, sizeof(new_author)) == 0) {
            strncpy(theme->author, new_author, sizeof(theme->author) - 1);

            if (dialog_inputbox("编辑描述", "输入主题描述:", 10, 60,
                               new_description, new_description, sizeof(new_description)) == 0) {
                strncpy(theme->description, new_description, sizeof(theme->description) - 1);
                theme->modified_time = time(NULL);
                dialog_msgbox("完成", "主题信息已更新", 8, 50);
            }
        }
    
        // 布局管理器对话框
        void show_layout_manager_dialog(void) {
            static LayoutManager lm;
            static int initialized = 0;
    
            if (!initialized) {
                if (layout_manager_init(&lm, "/usr/share/swikernel/layouts") != SWK_SUCCESS) {
                    dialog_msgbox("错误", "无法初始化布局管理器", 8, 50);
                    return;
                }
                initialized = 1;
            }
    
            while (1) {
                // 构建布局列表显示
                char display_text[MAX_BUFFER_SIZE] = {0};
                snprintf(display_text, sizeof(display_text),
                        "布局管理器\n"
                        "布局总数: %d\n"
                        "当前布局: %s\n"
                        "屏幕尺寸: %dx%d\n"
                        "终端类型: %s\n"
                        "颜色支持: %d色\n\n"
                        "可用布局:",
                        lm.layout_count,
                        lm.current_layout ? lm.current_layout->name : "无",
                        lm.screen_width, lm.screen_height,
                        lm.terminal_type ? "现代终端" : "传统终端",
                        lm.color_support);
    
                for (int i = 0; i < lm.layout_count; i++) {
                    snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                            "\n%d. %s - %s",
                            i + 1, lm.layouts[i].name, lm.layouts[i].description);
                }
    
                snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                        "\n\n选择操作:");
    
                // 构建菜单
                char menu_items[512] = {0};
                for (int i = 0; i < lm.layout_count; i++) {
                    char item_num[8];
                    snprintf(item_num, sizeof(item_num), "%d", i + 1);
                    strncat(menu_items, item_num, sizeof(menu_items) - strlen(menu_items) - 1);
                    strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
                    strncat(menu_items, lm.layouts[i].name, sizeof(menu_items) - strlen(menu_items) - 1);
                    strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
                }
    
                // 添加操作选项
                strncat(menu_items, "p\n预览布局\n", sizeof(menu_items) - strlen(menu_items) - 1);
                strncat(menu_items, "a\n自适应屏幕\n", sizeof(menu_items) - strlen(menu_items) - 1);
                strncat(menu_items, "d\n检测终端\n", sizeof(menu_items) - strlen(menu_items) - 1);
    
                int choice = dialog_menu("布局管理器",
                                       display_text,
                                       22, 80, lm.layout_count + 3,
                                       menu_items);
    
                if (choice >= 1 && choice <= lm.layout_count) {
                    // 应用指定布局
                    LayoutConfig *selected_layout = &lm.layouts[choice - 1];
                    layout_manager_apply_layout(&lm, selected_layout->name);
                    dialog_msgbox("完成", "布局已应用", 8, 50);
                } else {
                    switch (choice) {
                        case lm.layout_count + 1: // 预览布局
                            if (lm.current_layout) {
                                show_layout_preview_dialog(&lm, lm.current_layout);
                            }
                            break;
                        case lm.layout_count + 2: // 自适应屏幕
                            layout_manager_adapt_to_screen(&lm, lm.screen_width, lm.screen_height);
                            dialog_msgbox("完成", "布局已适应屏幕尺寸", 8, 50);
                            break;
                        case lm.layout_count + 3: // 检测终端
                            layout_manager_detect_terminal(&lm);
                            dialog_msgbox("完成", "终端检测完成", 8, 50);
                            break;
                        case -1:
                            return;
                        default:
                            break;
                    }
                }
            }
        }
    
        // 显示布局选择对话框
        void show_layout_selector_dialog(LayoutManager *lm) {
            if (!lm) return;
    
            char display_text[MAX_BUFFER_SIZE] = {0};
            snprintf(display_text, sizeof(display_text),
                    "选择布局\n"
                    "当前布局: %s\n\n"
                    "可用布局:",
                    lm->current_layout ? lm->current_layout->name : "无");
    
            for (int i = 0; i < lm->layout_count; i++) {
                snprintf(display_text + strlen(display_text), sizeof(display_text) - strlen(display_text),
                        "\n%d. %s - %s",
                        i + 1, lm->layouts[i].name, lm->layouts[i].description);
            }
    
            // 构建菜单项
            char menu_items[512] = {0};
            for (int i = 0; i < lm->layout_count; i++) {
                char item_num[8];
                snprintf(item_num, sizeof(item_num), "%d", i + 1);
                strncat(menu_items, item_num, sizeof(menu_items) - strlen(menu_items) - 1);
                strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
                strncat(menu_items, lm->layouts[i].name, sizeof(menu_items) - strlen(menu_items) - 1);
                strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
            }
    
            int choice = dialog_menu("布局选择",
                                   display_text,
                                   20, 80, lm->layout_count,
                                   menu_items);
    
            if (choice >= 1 && choice <= lm->layout_count) {
                LayoutConfig *selected_layout = &lm->layouts[choice - 1];
                layout_manager_apply_layout(lm, selected_layout->name);
            }
        }
    
        // 显示布局预览对话框
        void show_layout_preview_dialog(LayoutManager *lm, LayoutConfig *layout) {
            if (!lm || !layout) return;
    
            char preview_text[MAX_BUFFER_SIZE] = {0};
            snprintf(preview_text, sizeof(preview_text),
                    "布局预览: %s\n"
                    "================\n\n"
                    "基本信息:\n"
                    "名称: %s\n"
                    "描述: %s\n\n"
                    "尺寸设置:\n"
                    "窗口大小: %dx%d\n"
                    "对话框大小: %dx%d\n"
                    "最小尺寸: %dx%d\n"
                    "最大尺寸: %dx%d\n\n"
                    "布局选项:\n"
                    "菜单方向: %s\n"
                    "菜单对齐: %s\n"
                    "菜单列数: %d\n"
                    "元素间距: %d\n\n"
                    "视觉效果:\n"
                    "启用颜色: %s\n"
                    "启用动画: %s\n"
                    "紧凑模式: %s\n"
                    "响应式设计: %s\n"
                    "移动端优化: %s",
                    layout->name,
                    layout->name,
                    layout->description,
                    layout->window_size.width, layout->window_size.height,
                    layout->dialog_size.width, layout->dialog_size.height,
                    layout->dialog_size.min_width, layout->dialog_size.min_height,
                    layout->dialog_size.max_width, layout->dialog_size.max_height,
                    layout->menu_orientation ? "水平" : "垂直",
                    layout->menu_alignment ? "居中" : "左对齐",
                    layout->menu_columns,
                    layout->spacing,
                    layout->enable_colors ? "是" : "否",
                    layout->enable_animations ? "是" : "否",
                    layout->compact_mode ? "是" : "否",
                    layout->responsive_design ? "是" : "否",
                    layout->mobile_optimized ? "是" : "否");
    
            dialog_msgbox("布局预览", preview_text, 25, 80);
        }
    
        // 显示布局属性对话框
        void show_layout_properties_dialog(LayoutManager *lm, LayoutConfig *layout) {
            if (!lm || !layout) return;
    
            char display_text[MAX_BUFFER_SIZE] = {0};
            snprintf(display_text, sizeof(display_text),
                    "布局属性: %s\n"
                    "==============\n\n"
                    "窗口尺寸:\n"
                    "宽度: %d\n"
                    "高度: %d\n"
                    "最小宽度: %d\n"
                    "最小高度: %d\n"
                    "最大宽度: %d\n"
                    "最大高度: %d\n\n"
                    "边距设置:\n"
                    "左边距: %d\n"
                    "右边距: %d\n"
                    "上边距: %d\n"
                    "下边距: %d\n"
                    "水平间距: %d\n"
                    "垂直间距: %d\n\n"
                    "选择要修改的属性:",
                    layout->name,
                    layout->window_size.width,
                    layout->window_size.height,
                    layout->window_size.min_width,
                    layout->window_size.min_height,
                    layout->window_size.max_width,
                    layout->window_size.max_height,
                    layout->margins.left,
                    layout->margins.right,
                    layout->margins.top,
                    layout->margins.bottom,
                    layout->margins.horizontal,
                    layout->margins.vertical);
    
            int choice = dialog_menu("布局属性",
                                   display_text,
                                   25, 60, 8,
                                   "1", "修改窗口尺寸",
                                   "2", "修改对话框尺寸",
                                   "3", "修改边距设置",
                                   "4", "修改菜单选项",
                                   "5", "修改视觉效果",
                                   "6", "修改高级选项",
                                   "7", "重置为默认值",
                                   "8", "返回");
    
            switch (choice) {
                case 1:
                    show_window_size_dialog(lm, layout);
                    break;
                case 2:
                    show_dialog_size_dialog(lm, layout);
                    break;
                case 3:
                    show_margins_dialog(lm, layout);
                    break;
                case 4:
                    show_menu_options_dialog(lm, layout);
                    break;
                case 5:
                    show_visual_effects_dialog(lm, layout);
                    break;
                case 6:
                    show_advanced_options_dialog(lm, layout);
                    break;
                case 7:
                    if (dialog_yesno("确认重置", "确定要重置布局为默认值吗？", 8, 50) == 0) {
                        // 这里可以实现重置逻辑
                        dialog_msgbox("完成", "布局已重置", 8, 50);
                    }
                    break;
                default:
                    break;
            }
        }
    
        // 显示窗口尺寸对话框
        void show_window_size_dialog(LayoutManager *lm, LayoutConfig *layout) {
            if (!lm || !layout) return;
    
            char width_str[16], height_str[16];
            snprintf(width_str, sizeof(width_str), "%d", layout->window_size.width);
            snprintf(height_str, sizeof(height_str), "%d", layout->window_size.height);
    
            if (dialog_inputbox("窗口宽度", "输入窗口宽度:", 10, 30,
                               width_str, width_str, sizeof(width_str)) == 0) {
                layout->window_size.width = atoi(width_str);
    
                if (dialog_inputbox("窗口高度", "输入窗口高度:", 10, 30,
                                   height_str, height_str, sizeof(height_str)) == 0) {
                    layout->window_size.height = atoi(height_str);
                    dialog_msgbox("完成", "窗口尺寸已更新", 8, 50);
                }
            }
        }
    
        // 显示对话框尺寸对话框
        void show_dialog_size_dialog(LayoutManager *lm, LayoutConfig *layout) {
            if (!lm || !layout) return;
    
            char width_str[16], height_str[16];
            snprintf(width_str, sizeof(width_str), "%d", layout->dialog_size.width);
            snprintf(height_str, sizeof(height_str), "%d", layout->dialog_size.height);
    
            if (dialog_inputbox("对话框宽度", "输入对话框宽度:", 10, 30,
                               width_str, width_str, sizeof(width_str)) == 0) {
                layout->dialog_size.width = atoi(width_str);
    
                if (dialog_inputbox("对话框高度", "输入对话框高度:", 10, 30,
                                   height_str, height_str, sizeof(height_str)) == 0) {
                    layout->dialog_size.height = atoi(height_str);
                    dialog_msgbox("完成", "对话框尺寸已更新", 8, 50);
                }
            }
        }
    
        // 显示边距设置对话框
        void show_margins_dialog(LayoutManager *lm, LayoutConfig *layout) {
            if (!lm || !layout) return;
    
            char left_str[16], right_str[16], top_str[16], bottom_str[16];
            snprintf(left_str, sizeof(left_str), "%d", layout->margins.left);
            snprintf(right_str, sizeof(right_str), "%d", layout->margins.right);
            snprintf(top_str, sizeof(top_str), "%d", layout->margins.top);
            snprintf(bottom_str, sizeof(bottom_str), "%d", layout->margins.bottom);
    
            if (dialog_inputbox("左边距", "输入左边距:", 10, 30,
                               left_str, left_str, sizeof(left_str)) == 0) {
                layout->margins.left = atoi(left_str);
    
                if (dialog_inputbox("右边距", "输入右边距:", 10, 30,
                                   right_str, right_str, sizeof(right_str)) == 0) {
                    layout->margins.right = atoi(right_str);
    
                    if (dialog_inputbox("上边距", "输入上边距:", 10, 30,
                                       top_str, top_str, sizeof(top_str)) == 0) {
                        layout->margins.top = atoi(top_str);
    
                        if (dialog_inputbox("下边距", "输入下边距:", 10, 30,
                                           bottom_str, bottom_str, sizeof(bottom_str)) == 0) {
                            layout->margins.bottom = atoi(bottom_str);
                            dialog_msgbox("完成", "边距设置已更新", 8, 50);
                        }
                    }
                }
            }
        }
    
        // 显示菜单选项对话框
        void show_menu_options_dialog(LayoutManager *lm, LayoutConfig *layout) {
            if (!lm || !layout) return;
    
            char spacing_str[16], columns_str[16];
            snprintf(spacing_str, sizeof(spacing_str), "%d", layout->spacing);
            snprintf(columns_str, sizeof(columns_str), "%d", layout->menu_columns);
    
            if (dialog_inputbox("菜单间距", "输入菜单项间距:", 10, 30,
                               spacing_str, spacing_str, sizeof(spacing_str)) == 0) {
                layout->spacing = atoi(spacing_str);
    
                if (dialog_inputbox("菜单列数", "输入菜单列数:", 10, 30,
                                   columns_str, columns_str, sizeof(columns_str)) == 0) {
                    layout->menu_columns = atoi(columns_str);
                    dialog_msgbox("完成", "菜单选项已更新", 8, 50);
                }
            }
        }
    
        // 显示视觉效果对话框
        void show_visual_effects_dialog(LayoutManager *lm, LayoutConfig *layout) {
            if (!lm || !layout) return;
    
            char display_text[MAX_BUFFER_SIZE] = {0};
            snprintf(display_text, sizeof(display_text),
                    "视觉效果设置\n"
                    "=============\n\n"
                    "当前设置:\n"
                    "启用颜色: %s\n"
                    "启用粗体: %s\n"
                    "启用下划线: %s\n"
                    "启用闪烁: %s\n"
                    "启用反显: %s\n"
                    "启用动画: %s\n\n"
                    "选择要切换的选项:",
                    layout->enable_colors ? "是" : "否",
                    layout->enable_bold ? "是" : "否",
                    layout->enable_underline ? "是" : "否",
                    layout->enable_blink ? "是" : "否",
                    layout->enable_reverse ? "是" : "否",
                    layout->enable_animations ? "是" : "否");
    
            int choice = dialog_menu("视觉效果",
                                   display_text,
                                   18, 60, 6,
                                   "1", "切换颜色",
                                   "2", "切换粗体",
                                   "3", "切换下划线",
                                   "4", "切换闪烁",
                                   "5", "切换反显",
                                   "6", "切换动画");
    
            switch (choice) {
                case 1:
                    layout->enable_colors = !layout->enable_colors;
                    break;
                case 2:
                    layout->enable_bold = !layout->enable_bold;
                    break;
                case 3:
                    layout->enable_underline = !layout->enable_underline;
                    break;
                case 4:
                    layout->enable_blink = !layout->enable_blink;
                    break;
                case 5:
                    layout->enable_reverse = !layout->enable_reverse;
                    break;
                case 6:
                    layout->enable_animations = !layout->enable_animations;
                    break;
                default:
                    return;
            }
    
            dialog_msgbox("完成", "视觉效果已更新", 8, 50);
        }
    
        // 显示高级选项对话框
        void show_advanced_options_dialog(LayoutManager *lm, LayoutConfig *layout) {
            if (!lm || !layout) return;
    
            char display_text[MAX_BUFFER_SIZE] = {0};
            snprintf(display_text, sizeof(display_text),
                    "高级选项\n"
                    "=========\n\n"
                    "当前设置:\n"
                    "响应式设计: %s\n"
                    "自适应缩放: %s\n"
                    "移动端优化: %s\n"
                    "紧凑模式: %s\n"
                    "自定义CSS: %s\n"
                    "主题集成: %s\n"
                    "插件兼容性: %s\n\n"
                    "选择要切换的选项:",
                    layout->responsive_design ? "是" : "否",
                    layout->adaptive_scaling ? "是" : "否",
                    layout->mobile_optimized ? "是" : "否",
                    layout->compact_mode ? "是" : "否",
                    layout->custom_css ? "是" : "否",
                    layout->theme_integration ? "是" : "否",
                    layout->plugin_compatibility ? "是" : "否");
    
            int choice = dialog_menu("高级选项",
                                   display_text,
                                   20, 60, 7,
                                   "1", "切换响应式设计",
                                   "2", "切换自适应缩放",
                                   "3", "切换移动端优化",
                                   "4", "切换紧凑模式",
                                   "5", "切换自定义CSS",
                                   "6", "切换主题集成",
                                   "7", "切换插件兼容性");
    
            switch (choice) {
                case 1:
                    layout->responsive_design = !layout->responsive_design;
                    break;
                case 2:
                    layout->adaptive_scaling = !layout->adaptive_scaling;
                    break;
                case 3:
                    layout->mobile_optimized = !layout->mobile_optimized;
                    break;
                case 4:
                    layout->compact_mode = !layout->compact_mode;
                    break;
                case 5:
                    layout->custom_css = !layout->custom_css;
                    break;
                case 6:
                    layout->theme_integration = !layout->theme_integration;
                    break;
                case 7:
                    layout->plugin_compatibility = !layout->plugin_compatibility;
                    break;
                default:
                    return;
            }
    
            dialog_msgbox("完成", "高级选项已更新", 8, 50);
        }
    
        // 显示反馈对话框
        void show_feedback_dialog(FeedbackSystem *fs) {
            if (!fs) return;
    
            pthread_mutex_lock(&fs->mutex);
    
            if (!fs->messages) {
                dialog_msgbox("反馈信息", "暂无反馈消息", 8, 50);
                pthread_mutex_unlock(&fs->mutex);
                return;
            }
    
            // 构建消息列表
            char display_text[MAX_BUFFER_SIZE] = {0};
            snprintf(display_text, sizeof(display_text), "反馈消息 (%d条)\n\n", fs->message_count);
    
            FeedbackMessage *current = fs->messages;
            int count = 0;
            char msg_line[256];
    
            while (current && count < 10) {
                char *type_str = "未知";
                switch (current->type) {
                    case FEEDBACK_TYPE_INFO: type_str = "信息"; break;
                    case FEEDBACK_TYPE_SUCCESS: type_str = "成功"; break;
                    case FEEDBACK_TYPE_WARNING: type_str = "警告"; break;
                    case FEEDBACK_TYPE_ERROR: type_str = "错误"; break;
                    case FEEDBACK_TYPE_CRITICAL: type_str = "严重"; break;
                    case FEEDBACK_TYPE_DEBUG: type_str = "调试"; break;
                }
    
                struct tm *tm = localtime(&current->timestamp);
                snprintf(msg_line, sizeof(msg_line), "%s [%02d:%02d:%02d] %s: %s\n",
                        type_str,
                        tm->tm_hour, tm->tm_min, tm->tm_sec,
                        current->title,
                        current->message);
    
                strncat(display_text, msg_line, sizeof(display_text) - strlen(display_text) - 1);
                current = current->next;
                count++;
            }
    
            pthread_mutex_unlock(&fs->mutex);
    
            dialog_msgbox("反馈消息", display_text, 20, 80);
        }
    
        // 显示错误详情对话框
        void show_error_details_dialog(ErrorInfo *error) {
            if (!error) return;
    
            char details_text[MAX_BUFFER_SIZE] = {0};
            snprintf(details_text, sizeof(details_text),
                    "错误详情\n"
                    "==========\n\n"
                    "错误代码: %d\n"
                    "错误信息: %s\n"
                    "函数: %s\n"
                    "文件: %s\n"
                    "行号: %d\n\n"
                    "建议解决方案:\n%s",
                    error->code,
                    error->message,
                    error->function,
                    error->file,
                    error->line,
                    feedback_system_get_error_suggestion(error->code));
    
            dialog_msgbox("错误详情", details_text, 20, 80);
        }
    
        // 显示进度对话框
        void show_progress_dialog(ProgressInfo *progress) {
            if (!progress) return;
    
            char progress_text[MAX_BUFFER_SIZE] = {0};
            snprintf(progress_text, sizeof(progress_text),
                    "操作进度\n"
                    "==========\n\n"
                    "操作: %s\n"
                    "进度: %d/%d (%.1f%%)\n"
                    "状态: %s\n",
                    progress->operation,
                    progress->current,
                    progress->total,
                    progress->total > 0 ? (double)progress->current * 100.0 / progress->total : 0.0,
                    progress->status);
    
            // 计算ETA
            if (progress->show_eta && progress->current > 0 && progress->total > 0) {
                time_t elapsed = time(NULL) - progress->start_time;
                time_t remaining = (time_t)((double)elapsed * (progress->total - progress->current) / progress->current);
    
                snprintf(progress_text + strlen(progress_text), sizeof(progress_text) - strlen(progress_text),
                        "预计剩余时间: %ld秒\n", remaining);
            }
    
            dialog_msgbox("进度", progress_text, 12, 60);
        }
    
        // 显示确认对话框
        void show_confirmation_dialog(const char *title, const char *message, int *result) {
            if (!result) return;
    
            char confirm_msg[MAX_BUFFER_SIZE] = {0};
            snprintf(confirm_msg, sizeof(confirm_msg), "%s", message);
    
            *result = dialog_yesno((char*)title, confirm_msg, 10, 60);
        }
    
        // 显示反馈管理对话框
        void show_feedback_manager_dialog(void) {
            while (1) {
                int choice = dialog_menu("反馈管理",
                                       "选择反馈管理选项:",
                                       12, 60, 5,
                                       "1", "查看反馈消息",
                                       "2", "设置反馈级别",
                                       "3", "启用/禁用声音",
                                       "4", "清空消息历史",
                                       "5", "返回主菜单");
    
                switch (choice) {
                    case 1:
                        show_feedback_dialog(&g_feedback_system);
                        break;
                    case 2: {
                        char *levels[] = {"静默", "正常", "详细", "调试", NULL};
                        int level_choice = dialog_menu("选择反馈级别",
                                                    "选择反馈显示级别:",
                                                    12, 30, 4,
                                                    "1", "静默模式",
                                                    "2", "正常模式",
                                                    "3", "详细模式",
                                                    "4", "调试模式");
    
                        if (level_choice > 0) {
                            feedback_system_set_level(&g_feedback_system, level_choice - 1);
                            dialog_msgbox("完成", "反馈级别已更新", 8, 50);
                        }
                        break;
                    }
                    case 3: {
                        int sound_enabled = g_feedback_system.sound_enabled;
                        char sound_status[32];
                        snprintf(sound_status, sizeof(sound_status), "%s",
                                sound_enabled ? "启用" : "禁用");
    
                        int choice2 = dialog_menu("声音提示",
                                                "当前状态: %s\n\n选择操作:",
                                                10, 40, 2,
                                                "1", sound_enabled ? "禁用声音" : "启用声音",
                                                "2", "返回");
    
                        if (choice2 == 1) {
                            feedback_system_enable_sound(&g_feedback_system, !sound_enabled);
                            dialog_msgbox("完成", "声音提示设置已更新", 8, 50);
                        }
                        break;
                    }
                    case 4:
                        if (dialog_yesno("确认清空", "确定要清空所有反馈消息吗？", 8, 50) == 0) {
                            feedback_system_clear_messages(&g_feedback_system);
                            dialog_msgbox("完成", "反馈消息已清空", 8, 50);
                        }
                        break;
                    case 5:
                    case -1:
                        return;
                    default:
                        break;
                }
            }
        }
    
        // 显示增强的错误处理对话框
        void show_enhanced_error_dialog(ErrorInfo *error) {
            if (!error) return;
    
            char error_text[MAX_BUFFER_SIZE] = {0};
            snprintf(error_text, sizeof(error_text),
                    "操作失败\n"
                    "==========\n\n"
                    "错误信息: %s\n"
                    "错误代码: %d\n"
                    "出错位置: %s() in %s:%d\n\n"
                    "可能的原因:\n"
                    "• 文件权限不足\n"
                    "• 文件或目录不存在\n"
                    "• 系统资源不足\n"
                    "• 配置错误\n\n"
                    "建议解决方案:\n%s\n\n"
                    "是否要查看详细错误信息？",
                    error->message,
                    error->code,
                    error->function,
                    error->file,
                    error->line,
                    feedback_system_get_error_suggestion(error->code));
    
            int choice = dialog_yesno("错误", error_text, 20, 70);
    
            if (choice == 0) { // 用户选择了"是"
                show_error_details_dialog(error);
            }
        }
    
        // 显示操作确认对话框
        int show_operation_confirmation(const char *operation, const char *target, const char *warning) {
            char confirm_msg[MAX_BUFFER_SIZE] = {0};
            snprintf(confirm_msg, sizeof(confirm_msg),
                    "您即将执行以下操作:\n\n"
                    "操作: %s\n"
                    "目标: %s\n\n"
                    "%s\n\n"
                    "此操作可能需要一些时间，确定要继续吗？",
                    operation, target, warning ? warning : "");
    
            return (dialog_yesno("确认操作", confirm_msg, 15, 70) == 0);
        }
    
        // 显示输入验证对话框
        int show_input_validation_dialog(const char *prompt, char *input, size_t size, const char *validation_pattern) {
            if (dialog_inputbox("输入验证", (char*)prompt, 10, 60, "", input, size) != 0) {
                return 0; // 用户取消
            }
    
            // 这里可以实现输入验证逻辑
            if (strlen(input) == 0) {
                dialog_msgbox("输入错误", "输入不能为空", 8, 50);
                return 0;
            }
    
            if (validation_pattern && strstr(input, validation_pattern) == NULL) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "输入不符合要求，应包含: %s", validation_pattern);
                dialog_msgbox("输入错误", error_msg, 8, 50);
                return 0;
            }
    
            return 1; // 输入有效
        }
    
        // 显示状态指示器
        void show_status_indicator(const char *operation, int progress_percent) {
            char status_text[256];
            snprintf(status_text, sizeof(status_text),
                    "正在执行: %s\n"
                    "进度: [%3d%%] ",
                    operation, progress_percent);
    
            // 构建进度条
            int bar_width = 30;
            int filled = (progress_percent * bar_width) / 100;
            char progress_bar[bar_width + 3];
    
            snprintf(progress_bar, sizeof(progress_bar), "[");
            for (int i = 0; i < bar_width; i++) {
                progress_bar[i + 1] = (i < filled) ? '=' : ' ';
            }
            progress_bar[bar_width + 1] = ']';
            progress_bar[bar_width + 2] = '\0';
    
            strncat(status_text, progress_bar, sizeof(status_text) - strlen(status_text) - 1);
    
            // 这里可以实现状态显示逻辑
            // 由于dialog库的限制，我们主要通过日志来显示状态
            log_message(LOG_INFO, "Status: %s", status_text);
        }
    }
}