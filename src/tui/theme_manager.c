#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include "theme_manager.h"
#include "logger.h"

// 初始化主题管理器
int theme_manager_init(ThemeManager *tm, const char *theme_dir) {
    if (!tm || !theme_dir) {
        return SWK_ERROR_INVALID_PARAM;
    }

    memset(tm, 0, sizeof(ThemeManager));
    strncpy(tm->theme_dir, theme_dir, sizeof(tm->theme_dir) - 1);
    tm->auto_apply = 1;

    // 创建主题目录（如果不存在）
    struct stat st = {0};
    if (stat(theme_dir, &st) == -1) {
        if (mkdir(theme_dir, 0755) != 0) {
            log_message(LOG_ERROR, "Failed to create theme directory: %s", theme_dir);
            return SWK_ERROR_SYSTEM_CALL;
        }
    }

    // 创建内置主题
    if (theme_manager_create_builtin_themes(tm) != SWK_SUCCESS) {
        log_message(LOG_WARNING, "Failed to create builtin themes");
    }

    // 加载用户主题
    if (theme_manager_load_themes(tm) != SWK_SUCCESS) {
        log_message(LOG_WARNING, "Failed to load user themes");
    }

    log_message(LOG_DEBUG, "Theme manager initialized with %d themes", tm->theme_count);
    return SWK_SUCCESS;
}

// 清理主题管理器
void theme_manager_cleanup(ThemeManager *tm) {
    if (!tm) return;

    // 释放主题列表
    ThemeInfo *current = tm->themes;
    while (current) {
        ThemeInfo *next = current->next;
        if (!current->is_builtin) {
            free(current);
        }
        current = next;
    }

    tm->themes = NULL;
    tm->current_theme = NULL;
    tm->theme_count = 0;
}

// 创建内置主题
int theme_manager_create_builtin_themes(ThemeManager *tm) {
    if (!tm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 默认主题（浅色）
    ThemeInfo *default_theme = malloc(sizeof(ThemeInfo));
    if (!default_theme) return SWK_ERROR_OUT_OF_MEMORY;

    memset(default_theme, 0, sizeof(ThemeInfo));
    strcpy(default_theme->name, "default");
    strcpy(default_theme->version, "1.0");
    strcpy(default_theme->author, "SwiKernel Team");
    strcpy(default_theme->description, "默认浅色主题");
    default_theme->is_builtin = 1;
    default_theme->is_enabled = 1;
    default_theme->created_time = time(NULL);

    // 设置默认颜色
    default_theme->colors.background = 0;      // 默认背景
    default_theme->colors.foreground = 15;     // 白色前景
    default_theme->colors.border = 8;          // 灰色边框
    default_theme->colors.title = 12;          // 亮蓝色标题
    default_theme->colors.success = 10;        // 绿色成功
    default_theme->colors.warning = 11;        // 黄色警告
    default_theme->colors.error = 9;           // 红色错误
    default_theme->colors.info = 14;           // 青色信息

    // 添加到主题列表
    default_theme->next = tm->themes;
    tm->themes = default_theme;
    tm->current_theme = default_theme;
    tm->theme_count++;

    // 暗色主题
    ThemeInfo *dark_theme = malloc(sizeof(ThemeInfo));
    if (!dark_theme) return SWK_ERROR_OUT_OF_MEMORY;

    memset(dark_theme, 0, sizeof(ThemeInfo));
    strcpy(dark_theme->name, "dark");
    strcpy(dark_theme->version, "1.0");
    strcpy(dark_theme->author, "SwiKernel Team");
    strcpy(dark_theme->description, "暗色主题，适合夜间使用");
    dark_theme->is_builtin = 1;
    dark_theme->is_enabled = 0;
    dark_theme->created_time = time(NULL);

    // 设置暗色主题颜色
    dark_theme->colors.background = 0;         // 黑色背景
    dark_theme->colors.foreground = 15;        // 白色前景
    dark_theme->colors.border = 8;             // 灰色边框
    dark_theme->colors.title = 11;             // 黄色标题
    dark_theme->colors.success = 10;           // 绿色成功
    dark_theme->colors.warning = 14;           // 青色警告
    dark_theme->colors.error = 12;             // 亮红色错误
    dark_theme->colors.info = 13;              // 品红色信息

    // 添加到主题列表
    dark_theme->next = tm->themes;
    tm->themes = dark_theme;
    tm->theme_count++;

    log_message(LOG_DEBUG, "Created %d builtin themes", 2);
    return SWK_SUCCESS;
}

// 加载用户主题
int theme_manager_load_themes(ThemeManager *tm) {
    if (!tm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    DIR *dir;
    struct dirent *entry;

    dir = opendir(tm->theme_dir);
    if (!dir) {
        log_message(LOG_WARNING, "Cannot open theme directory: %s", tm->theme_dir);
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    while ((entry = readdir(dir)) != NULL) {
        // 只处理 .theme 文件
        if (strstr(entry->d_name, ".theme") == NULL) {
            continue;
        }

        char theme_path[MAX_PATH_LENGTH];
        snprintf(theme_path, sizeof(theme_path), "%s/%s", tm->theme_dir, entry->d_name);

        ThemeInfo *theme;
        if (theme_manager_load_from_file(tm, theme_path, &theme) == SWK_SUCCESS) {
            theme->is_builtin = 0;
            theme->next = tm->themes;
            tm->themes = theme;
            tm->theme_count++;
        }
    }

    closedir(dir);
    log_message(LOG_DEBUG, "Loaded %d user themes", tm->theme_count - 2);
    return SWK_SUCCESS;
}

// 应用主题
int theme_manager_apply_theme(ThemeManager *tm, const char *theme_name) {
    if (!tm || !theme_name) {
        return SWK_ERROR_INVALID_PARAM;
    }

    ThemeInfo *theme = tm->themes;
    while (theme) {
        if (strcmp(theme->name, theme_name) == 0) {
            tm->current_theme = theme;

            // 应用主题到UI
            if (tm->auto_apply) {
                theme_manager_apply_to_ui(tm);
            }

            log_message(LOG_INFO, "Applied theme: %s", theme_name);
            return SWK_SUCCESS;
        }
        theme = theme->next;
    }

    return SWK_ERROR_FILE_NOT_FOUND;
}

// 获取当前颜色
int theme_manager_get_color(ThemeManager *tm, const char *color_name, int *color_code) {
    if (!tm || !color_name || !color_code) {
        return SWK_ERROR_INVALID_PARAM;
    }

    if (!tm->current_theme) {
        *color_code = 15; // 默认白色
        return SWK_SUCCESS;
    }

    // 根据颜色名称返回对应的颜色代码
    if (strcmp(color_name, "background") == 0) {
        *color_code = tm->current_theme->colors.background;
    } else if (strcmp(color_name, "foreground") == 0) {
        *color_code = tm->current_theme->colors.foreground;
    } else if (strcmp(color_name, "border") == 0) {
        *color_code = tm->current_theme->colors.border;
    } else if (strcmp(color_name, "title") == 0) {
        *color_code = tm->current_theme->colors.title;
    } else if (strcmp(color_name, "success") == 0) {
        *color_code = tm->current_theme->colors.success;
    } else if (strcmp(color_name, "warning") == 0) {
        *color_code = tm->current_theme->colors.warning;
    } else if (strcmp(color_name, "error") == 0) {
        *color_code = tm->current_theme->colors.error;
    } else if (strcmp(color_name, "info") == 0) {
        *color_code = tm->current_theme->colors.info;
    } else {
        *color_code = tm->current_theme->colors.foreground; // 默认前景色
    }

    return SWK_SUCCESS;
}

// 获取颜色字符串
const char *theme_manager_get_color_string(ThemeManager *tm, const char *color_name) {
    static char color_str[16];
    int color_code;

    if (theme_manager_get_color(tm, color_name, &color_code) == SWK_SUCCESS) {
        snprintf(color_str, sizeof(color_str), "\033[%dm", color_code);
        return color_str;
    }

    return "\033[0m"; // 默认颜色
}

// 应用主题到UI
int theme_manager_apply_to_ui(ThemeManager *tm) {
    if (!tm || !tm->current_theme) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 这里可以实现将主题应用到dialog库的逻辑
    // 由于dialog库的限制，我们主要通过颜色代码来实现主题效果

    log_message(LOG_DEBUG, "Applied theme '%s' to UI", tm->current_theme->name);
    return SWK_SUCCESS;
}

// 解析颜色代码
int parse_color_code(const char *color_str) {
    if (!color_str) return 0;

    // 支持数字颜色代码
    if (isdigit(color_str[0])) {
        return atoi(color_str);
    }

    // 支持颜色名称
    if (strcasecmp(color_str, "black") == 0) return 0;
    if (strcasecmp(color_str, "red") == 0) return 1;
    if (strcasecmp(color_str, "green") == 0) return 2;
    if (strcasecmp(color_str, "yellow") == 0) return 3;
    if (strcasecmp(color_str, "blue") == 0) return 4;
    if (strcasecmp(color_str, "magenta") == 0) return 5;
    if (strcasecmp(color_str, "cyan") == 0) return 6;
    if (strcasecmp(color_str, "white") == 0) return 7;

    // 亮色
    if (strcasecmp(color_str, "bright_black") == 0 || strcasecmp(color_str, "gray") == 0) return 8;
    if (strcasecmp(color_str, "bright_red") == 0) return 9;
    if (strcasecmp(color_str, "bright_green") == 0) return 10;
    if (strcasecmp(color_str, "bright_yellow") == 0) return 11;
    if (strcasecmp(color_str, "bright_blue") == 0) return 12;
    if (strcasecmp(color_str, "bright_magenta") == 0) return 13;
    if (strcasecmp(color_str, "bright_cyan") == 0) return 14;
    if (strcasecmp(color_str, "bright_white") == 0) return 15;

    return 0; // 默认黑色
}

// 颜色代码转字符串
const char *color_code_to_string(int color_code) {
    switch (color_code) {
        case 0: return "black";
        case 1: return "red";
        case 2: return "green";
        case 3: return "yellow";
        case 4: return "blue";
        case 5: return "magenta";
        case 6: return "cyan";
        case 7: return "white";
        case 8: return "bright_black";
        case 9: return "bright_red";
        case 10: return "bright_green";
        case 11: return "bright_yellow";
        case 12: return "bright_blue";
        case 13: return "bright_magenta";
        case 14: return "bright_cyan";
        case 15: return "bright_white";
        default: return "unknown";
    }
}

// 获取颜色调色板
void get_color_palette(ColorDefinition *palette, int *count) {
    if (!palette || !count) return;

    ColorDefinition colors[] = {
        {"black", 0, "黑色"},
        {"red", 1, "红色"},
        {"green", 2, "绿色"},
        {"yellow", 3, "黄色"},
        {"blue", 4, "蓝色"},
        {"magenta", 5, "品红色"},
        {"cyan", 6, "青色"},
        {"white", 7, "白色"},
        {"bright_black", 8, "亮黑色"},
        {"bright_red", 9, "亮红色"},
        {"bright_green", 10, "亮绿色"},
        {"bright_yellow", 11, "亮黄色"},
        {"bright_blue", 12, "亮蓝色"},
        {"bright_magenta", 13, "亮品红色"},
        {"bright_cyan", 14, "亮青色"},
        {"bright_white", 15, "亮白色"}
    };

    *count = sizeof(colors) / sizeof(colors[0]);

    for (int i = 0; i < *count; i++) {
        memcpy(&palette[i], &colors[i], sizeof(ColorDefinition));
    }
}

// 从文件加载主题
int theme_manager_load_from_file(ThemeManager *tm, const char *file_path, ThemeInfo **theme) {
    if (!tm || !file_path || !theme) {
        return SWK_ERROR_INVALID_PARAM;
    }

    FILE *file = fopen(file_path, "r");
    if (!file) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    ThemeInfo *new_theme = malloc(sizeof(ThemeInfo));
    if (!new_theme) {
        fclose(file);
        return SWK_ERROR_OUT_OF_MEMORY;
    }

    memset(new_theme, 0, sizeof(ThemeInfo));

    // 设置默认值
    strcpy(new_theme->name, "unnamed");
    strcpy(new_theme->version, "1.0");
    strcpy(new_theme->author, "Unknown");
    strcpy(new_theme->description, "Custom theme");
    new_theme->is_builtin = 0;
    new_theme->is_enabled = 1;
    new_theme->created_time = time(NULL);
    new_theme->modified_time = time(NULL);

    // 设置默认颜色
    new_theme->colors.background = 0;
    new_theme->colors.foreground = 15;
    new_theme->colors.border = 8;
    new_theme->colors.title = 12;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // 移除换行符
        line[strcspn(line, "\n\r")] = '\0';

        // 跳过空行和注释
        if (strlen(line) == 0 || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // 解析键值对
        char *equal = strchr(line, '=');
        if (equal) {
            *equal = '\0';
            char *key = line;
            char *value = equal + 1;

            // 移除空格
            while (*key == ' ' || *key == '\t') key++;
            while (*value == ' ' || *value == '\t') value++;

            // 解析主题信息
            if (strcmp(key, "name") == 0) {
                strncpy(new_theme->name, value, sizeof(new_theme->name) - 1);
            } else if (strcmp(key, "version") == 0) {
                strncpy(new_theme->version, value, sizeof(new_theme->version) - 1);
            } else if (strcmp(key, "author") == 0) {
                strncpy(new_theme->author, value, sizeof(new_theme->author) - 1);
            } else if (strcmp(key, "description") == 0) {
                strncpy(new_theme->description, value, sizeof(new_theme->description) - 1);
            } else if (strcmp(key, "background") == 0) {
                new_theme->colors.background = parse_color_code(value);
            } else if (strcmp(key, "foreground") == 0) {
                new_theme->colors.foreground = parse_color_code(value);
            } else if (strcmp(key, "border") == 0) {
                new_theme->colors.border = parse_color_code(value);
            } else if (strcmp(key, "title") == 0) {
                new_theme->colors.title = parse_color_code(value);
            }
        }
    }

    fclose(file);
    *theme = new_theme;

    log_message(LOG_DEBUG, "Loaded theme from file: %s", file_path);
    return SWK_SUCCESS;
}

// 保存主题到文件
int theme_manager_save_to_file(ThemeManager *tm, ThemeInfo *theme, const char *file_path) {
    if (!tm || !theme || !file_path) {
        return SWK_ERROR_INVALID_PARAM;
    }

    FILE *file = fopen(file_path, "w");
    if (!file) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    fprintf(file, "# SwiKernel Theme File\n");
    fprintf(file, "# Generated by Theme Manager\n\n");

    fprintf(file, "name = %s\n", theme->name);
    fprintf(file, "version = %s\n", theme->version);
    fprintf(file, "author = %s\n", theme->author);
    fprintf(file, "description = %s\n\n", theme->description);

    fprintf(file, "# Color Settings\n");
    fprintf(file, "background = %s\n", color_code_to_string(theme->colors.background));
    fprintf(file, "foreground = %s\n", color_code_to_string(theme->colors.foreground));
    fprintf(file, "border = %s\n", color_code_to_string(theme->colors.border));
    fprintf(file, "title = %s\n", color_code_to_string(theme->colors.title));
    fprintf(file, "success = %s\n", color_code_to_string(theme->colors.success));
    fprintf(file, "warning = %s\n", color_code_to_string(theme->colors.warning));
    fprintf(file, "error = %s\n", color_code_to_string(theme->colors.error));
    fprintf(file, "info = %s\n", color_code_to_string(theme->colors.info));

    fclose(file);
    theme->modified_time = time(NULL);

    log_message(LOG_DEBUG, "Saved theme to file: %s", file_path);
    return SWK_SUCCESS;
}