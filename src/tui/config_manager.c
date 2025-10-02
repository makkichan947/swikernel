#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config_manager.h"
#include "logger.h"
#include "config_parser.h"

// 初始化配置管理器
int config_manager_init(ConfigManager *cm, const char *config_file_path) {
    if (!cm || !config_file_path) {
        return SWK_ERROR_INVALID_PARAM;
    }

    memset(cm, 0, sizeof(ConfigManager));
    strncpy(cm->config_file_path, config_file_path, sizeof(cm->config_file_path) - 1);

    return config_manager_load(cm);
}

// 清理配置管理器
void config_manager_cleanup(ConfigManager *cm) {
    if (!cm) return;

    // 释放配置项
    ConfigItem *current = cm->items;
    while (current) {
        ConfigItem *next = current->next;
        config_item_destroy(current);
        current = next;
    }

    cm->items = NULL;
    cm->item_count = 0;
    cm->modified = 0;
}

// 重新加载配置
int config_manager_reload(ConfigManager *cm) {
    return config_manager_load(cm);
}

// 加载配置文件
int config_manager_load(ConfigManager *cm) {
    if (!cm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    FILE *file = fopen(cm->config_file_path, "r");
    if (!file) {
        log_message(LOG_ERROR, "Cannot open config file: %s", cm->config_file_path);
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    // 清空现有配置项
    config_manager_cleanup(cm);

    char line[512];
    char current_section[64] = {0};
    ConfigItem *tail = NULL;

    while (fgets(line, sizeof(line), file)) {
        // 移除换行符和空格
        line[strcspn(line, "\n\r")] = '\0';

        // 跳过空行和注释
        if (strlen(line) == 0 || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // 检查是否为节标题 [section]
        if (line[0] == '[' && line[strlen(line) - 1] == ']') {
            line[strlen(line) - 1] = '\0'; // 移除末尾的 ]
            strncpy(current_section, line + 1, sizeof(current_section) - 1);
            continue;
        }

        // 解析键值对
        char *equal = strchr(line, '=');
        if (equal && strlen(current_section) > 0) {
            *equal = '\0';
            char *key = line;
            char *value = equal + 1;

            // 移除空格
            while (*key == ' ' || *key == '\t') key++;
            while (*value == ' ' || *value == '\t') value++;

            char *key_end = key + strlen(key) - 1;
            while (key_end > key && (*key_end == ' ' || *key_end == '\t')) {
                *key_end = '\0';
                key_end--;
            }

            // 创建配置项
            ConfigItem *item = config_item_create(current_section, key, value, value, "", CONFIG_TYPE_STRING);
            if (item) {
                if (!cm->items) {
                    cm->items = item;
                    tail = item;
                } else {
                    tail->next = item;
                    tail = item;
                }
                cm->item_count++;
            }
        }
    }

    fclose(file);
    log_message(LOG_DEBUG, "Loaded %d config items from %s", cm->item_count, cm->config_file_path);
    return SWK_SUCCESS;
}

// 保存配置文件
int config_manager_save(ConfigManager *cm) {
    if (!cm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    FILE *file = fopen(cm->config_file_path, "w");
    if (!file) {
        log_message(LOG_ERROR, "Cannot write config file: %s", cm->config_file_path);
        return SWK_ERROR_PERMISSION_DENIED;
    }

    char current_section[64] = {0};
    ConfigItem *current = cm->items;

    while (current) {
        // 检查是否需要写入节标题
        if (strcmp(current->section, current_section) != 0) {
            if (strlen(current_section) > 0) {
                fprintf(file, "\n");
            }
            fprintf(file, "[%s]\n", current->section);
            strncpy(current_section, current->section, sizeof(current_section) - 1);
        }

        // 写入键值对
        fprintf(file, "%s = %s\n", current->key, current->value);
        current = current->next;
    }

    fclose(file);
    cm->modified = 0;
    log_message(LOG_INFO, "Saved config to %s", cm->config_file_path);
    return SWK_SUCCESS;
}

// 获取配置项
ConfigItem *config_manager_get_item(ConfigManager *cm, const char *section, const char *key) {
    if (!cm || !section || !key) return NULL;

    ConfigItem *current = cm->items;
    while (current) {
        if (strcmp(current->section, section) == 0 && strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

// 设置配置值
int config_manager_set_value(ConfigManager *cm, const char *section, const char *key, const char *value) {
    if (!cm || !section || !key || !value) {
        return SWK_ERROR_INVALID_PARAM;
    }

    ConfigItem *item = config_manager_get_item(cm, section, key);
    if (!item) {
        // 创建新配置项
        item = config_item_create(section, key, value, value, "", CONFIG_TYPE_STRING);
        if (!item) return SWK_ERROR_OUT_OF_MEMORY;

        // 添加到链表末尾
        ConfigItem *tail = cm->items;
        if (!tail) {
            cm->items = item;
        } else {
            while (tail->next) tail = tail->next;
            tail->next = item;
        }
        cm->item_count++;
    } else {
        // 更新现有配置项
        strncpy(item->value, value, sizeof(item->value) - 1);
    }

    cm->modified = 1;
    return SWK_SUCCESS;
}

// 获取配置值
const char *config_manager_get_value(ConfigManager *cm, const char *section, const char *key) {
    ConfigItem *item = config_manager_get_item(cm, section, key);
    return item ? item->value : NULL;
}

// 重置为默认值
int config_manager_reset_to_default(ConfigManager *cm, const char *section, const char *key) {
    if (!cm || !section || !key) {
        return SWK_ERROR_INVALID_PARAM;
    }

    ConfigItem *item = config_manager_get_item(cm, section, key);
    if (!item) return SWK_ERROR_FILE_NOT_FOUND;

    strncpy(item->value, item->default_value, sizeof(item->value) - 1);
    cm->modified = 1;
    return SWK_SUCCESS;
}

// 创建配置项
ConfigItem *config_item_create(const char *section, const char *key, const char *value,
                              const char *default_value, const char *description, ConfigType type) {
    ConfigItem *item = malloc(sizeof(ConfigItem));
    if (!item) return NULL;

    memset(item, 0, sizeof(ConfigItem));

    if (section) strncpy(item->section, section, sizeof(item->section) - 1);
    if (key) strncpy(item->key, key, sizeof(item->key) - 1);
    if (value) strncpy(item->value, value, sizeof(item->value) - 1);
    if (default_value) strncpy(item->default_value, default_value, sizeof(item->default_value) - 1);
    if (description) strncpy(item->description, description, sizeof(item->description) - 1);

    item->type = type;
    return item;
}

// 销毁配置项
void config_item_destroy(ConfigItem *item) {
    if (!item) return;

    if (item->enum_values) {
        free(item->enum_values);
    }

    free(item);
}

// 配置类型转字符串
const char *config_type_to_string(ConfigType type) {
    switch (type) {
        case CONFIG_TYPE_BOOL: return "布尔值";
        case CONFIG_TYPE_INT: return "整数";
        case CONFIG_TYPE_STRING: return "字符串";
        case CONFIG_TYPE_ENUM: return "枚举";
        case CONFIG_TYPE_PATH: return "路径";
        case CONFIG_TYPE_FILE: return "文件";
        default: return "未知";
    }
}

// 字符串转配置类型
ConfigType string_to_config_type(const char *str) {
    if (!str) return CONFIG_TYPE_STRING;

    if (strcmp(str, "bool") == 0) return CONFIG_TYPE_BOOL;
    if (strcmp(str, "int") == 0) return CONFIG_TYPE_INT;
    if (strcmp(str, "string") == 0) return CONFIG_TYPE_STRING;
    if (strcmp(str, "enum") == 0) return CONFIG_TYPE_ENUM;
    if (strcmp(str, "path") == 0) return CONFIG_TYPE_PATH;
    if (strcmp(str, "file") == 0) return CONFIG_TYPE_FILE;

    return CONFIG_TYPE_STRING;
}

// 获取指定节的所有配置项
ConfigItem *config_manager_get_items_in_section(ConfigManager *cm, const char *section) {
    if (!cm || !section) return NULL;

    ConfigItem *result = NULL;
    ConfigItem *tail = NULL;
    ConfigItem *current = cm->items;

    while (current) {
        if (strcmp(current->section, section) == 0) {
            ConfigItem *item = malloc(sizeof(ConfigItem));
            if (item) {
                memcpy(item, current, sizeof(ConfigItem));
                item->next = NULL;

                if (!result) {
                    result = item;
                    tail = item;
                } else {
                    tail->next = item;
                    tail = item;
                }
            }
        }
        current = current->next;
    }

    return result;
}

// 验证配置项
int config_manager_validate_item(ConfigManager *cm, ConfigItem *item) {
    if (!cm || !item) return SWK_ERROR_INVALID_PARAM;

    switch (item->type) {
        case CONFIG_TYPE_BOOL:
            return (strcmp(item->value, "true") == 0 || strcmp(item->value, "false") == 0) ?
                   SWK_SUCCESS : SWK_ERROR_INVALID_PARAM;

        case CONFIG_TYPE_INT:
            // 检查是否为有效整数
            for (size_t i = 0; i < strlen(item->value); i++) {
                if (!isdigit(item->value[i]) && !(i == 0 && item->value[i] == '-')) {
                    return SWK_ERROR_INVALID_PARAM;
                }
            }
            return SWK_SUCCESS;

        case CONFIG_TYPE_PATH:
        case CONFIG_TYPE_FILE:
            // 基本路径验证
            return (strlen(item->value) > 0 && item->value[0] == '/') ?
                   SWK_SUCCESS : SWK_ERROR_INVALID_PARAM;

        default:
            return SWK_SUCCESS;
    }
}

// 验证所有配置项
int config_manager_validate_all(ConfigManager *cm) {
    if (!cm) return SWK_ERROR_INVALID_PARAM;

    ConfigItem *current = cm->items;
    int errors = 0;

    while (current) {
        if (config_manager_validate_item(cm, current) != SWK_SUCCESS) {
            log_message(LOG_ERROR, "Invalid config item: %s.%s = %s",
                       current->section, current->key, current->value);
            errors++;
        }
        current = current->next;
    }

    return errors == 0 ? SWK_SUCCESS : SWK_ERROR;
}