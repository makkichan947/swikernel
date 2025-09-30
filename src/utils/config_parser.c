// src/utils/config_parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "config_parser.h"
#include "logger.h"

// 加载配置文件
int load_config(SwikernelConfig *config) {
    char config_path[256];
    
    // 尝试多个配置文件位置
    const char *possible_paths[] = {
        "/etc/swikernel/swikernel.conf",
        "/usr/local/etc/swikernel.conf",
        "~/.config/swikernel.conf",
        "./swikernel.conf",
        NULL
    };
    
    FILE *file = NULL;
    for (int i = 0; possible_paths[i]; i++) {
        snprintf(config_path, sizeof(config_path), "%s", possible_paths[i]);
        
        // 处理家目录路径
        if (config_path[0] == '~') {
            const char *home = getenv("HOME");
            if (home) {
                char expanded_path[256];
                snprintf(expanded_path, sizeof(expanded_path), "%s%s", home, config_path + 1);
                file = fopen(expanded_path, "r");
            }
        } else {
            file = fopen(config_path, "r");
        }
        
        if (file) {
            log_message(LOG_DEBUG, "Loading config from: %s", config_path);
            break;
        }
    }
    
    if (!file) {
        log_message(LOG_WARNING, "Config file not found, using defaults");
        set_default_config(config);
        return -1;
    }
    
    char line[256];
    char section[64] = {0};
    int line_num = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        // 移除换行符和尾随空白
        line[strcspn(line, "\n")] = 0;
        char *trimmed = trim_whitespace(line);
        
        // 跳过空行和注释
        if (trimmed[0] == '#' || trimmed[0] == ';' || trimmed[0] == 0) {
            continue;
        }
        
        // 检查节头
        if (trimmed[0] == '[' && trimmed[strlen(trimmed)-1] == ']') {
            strncpy(section, trimmed + 1, strlen(trimmed) - 2);
            section[strlen(trimmed) - 2] = 0;
            continue;
        }
        
        // 解析键值对
        char *key = trimmed;
        char *value = strchr(trimmed, '=');
        if (!value) {
            log_message(LOG_WARNING, "Invalid config line %d: %s", line_num, line);
            continue;
        }
        *value = 0;
        value++;
        
        // 去除键值前后空白
        key = trim_whitespace(key);
        value = trim_whitespace(value);
        
        // 根据节和键设置配置值
        if (set_config_value(config, section, key, value) != 0) {
            log_message(LOG_WARNING, "Unknown config key: %s.%s", section, key);
        }
    }
    
    fclose(file);
    log_message(LOG_INFO, "Configuration loaded successfully from %s", config_path);
    return 0;
}

// 保存配置到文件
int save_config(const SwikernelConfig *config, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        log_message(LOG_ERROR, "Failed to open config file for writing: %s", filename);
        return -1;
    }
    
    fprintf(file, "# SwiKernel Configuration File\n\n");
    
    // 日志配置
    fprintf(file, "[logging]\n");
    fprintf(file, "level = %d\n", config->log_level);
    fprintf(file, "file = %s\n", config->log_file);
    fprintf(file, "max_size = %d\n", config->log_max_size);
    fprintf(file, "rotate = %d\n\n", config->log_rotate);
    
    // 内核配置
    fprintf(file, "[kernel]\n");
    fprintf(file, "default_source_dir = %s\n", config->default_source_dir);
    fprintf(file, "backup_enabled = %d\n", config->backup_enabled);
    fprintf(file, "auto_dependencies = %d\n", config->auto_dependencies);
    fprintf(file, "parallel_compilation = %d\n\n", config->parallel_compilation);
    
    // 安装配置
    fprintf(file, "[installation]\n");
    fprintf(file, "keep_source = %d\n", config->keep_source);
    fprintf(file, "auto_reboot = %d\n", config->auto_reboot);
    fprintf(file, "timeout = %d\n\n", config->install_timeout);
    
    // UI配置
    fprintf(file, "[ui]\n");
    fprintf(file, "color_scheme = %s\n", config->color_scheme);
    fprintf(file, "auto_complete = %d\n", config->auto_complete);
    fprintf(file, "show_progress = %d\n", config->show_progress);
    
    fclose(file);
    log_message(LOG_INFO, "Configuration saved to %s", filename);
    return 0;
}

// 设置默认配置
void set_default_config(SwikernelConfig *config) {
    memset(config, 0, sizeof(SwikernelConfig));
    
    strcpy(config->log_file, "/var/log/swikernel.log");
    config->log_level = LOG_INFO;
    config->log_max_size = 10 * 1024 * 1024; // 10MB
    config->log_rotate = 1;
    
    strcpy(config->default_source_dir, "/usr/src");
    config->backup_enabled = 1;
    config->auto_dependencies = 1;
    config->parallel_compilation = 1;
    
    config->keep_source = 0;
    config->auto_reboot = 0;
    config->install_timeout = 3600;
    
    strcpy(config->color_scheme, "dark");
    config->auto_complete = 1;
    config->show_progress = 1;
    
    log_message(LOG_DEBUG, "Default configuration set");
}

// 设置配置值
int set_config_value(SwikernelConfig *config, const char *section, const char *key, const char *value) {
    if (strcmp(section, "logging") == 0) {
        if (strcmp(key, "level") == 0) {
            config->log_level = atoi(value);
        } else if (strcmp(key, "file") == 0) {
            strncpy(config->log_file, value, sizeof(config->log_file) - 1);
        } else if (strcmp(key, "max_size") == 0) {
            config->log_max_size = atoi(value);
        } else if (strcmp(key, "rotate") == 0) {
            config->log_rotate = atoi(value);
        } else {
            return -1;
        }
    } else if (strcmp(section, "kernel") == 0) {
        if (strcmp(key, "default_source_dir") == 0) {
            strncpy(config->default_source_dir, value, sizeof(config->default_source_dir) - 1);
        } else if (strcmp(key, "backup_enabled") == 0) {
            config->backup_enabled = atoi(value);
        } else if (strcmp(key, "auto_dependencies") == 0) {
            config->auto_dependencies = atoi(value);
        } else if (strcmp(key, "parallel_compilation") == 0) {
            config->parallel_compilation = atoi(value);
        } else {
            return -1;
        }
    } else if (strcmp(section, "installation") == 0) {
        if (strcmp(key, "keep_source") == 0) {
            config->keep_source = atoi(value);
        } else if (strcmp(key, "auto_reboot") == 0) {
            config->auto_reboot = atoi(value);
        } else if (strcmp(key, "timeout") == 0) {
            config->install_timeout = atoi(value);
        } else {
            return -1;
        }
    } else if (strcmp(section, "ui") == 0) {
        if (strcmp(key, "color_scheme") == 0) {
            strncpy(config->color_scheme, value, sizeof(config->color_scheme) - 1);
        } else if (strcmp(key, "auto_complete") == 0) {
            config->auto_complete = atoi(value);
        } else if (strcmp(key, "show_progress") == 0) {
            config->show_progress = atoi(value);
        } else {
            return -1;
        }
    } else {
        return -1;
    }
    
    return 0;
}

// 去除字符串前后空白
char* trim_whitespace(char *str) {
    char *end;
    
    // 去除前导空白
    while (isspace((unsigned char)*str)) {
        str++;
    }
    
    if (*str == 0) {
        return str;
    }
    
    // 去除尾部空白
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    
    // 写入终止空字符
    *(end + 1) = 0;
    
    return str;
}