#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "../common_defs.h"

// 配置项类型
typedef enum {
    CONFIG_TYPE_BOOL,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_ENUM,
    CONFIG_TYPE_PATH,
    CONFIG_TYPE_FILE
} ConfigType;

// 配置项结构
typedef struct ConfigItem {
    char section[64];
    char key[64];
    char value[256];
    char default_value[256];
    char description[256];
    ConfigType type;
    char *enum_values; // 枚举值列表，用逗号分隔
    int min_value;     // 最小值（用于整数类型）
    int max_value;     // 最大值（用于整数类型）
    struct ConfigItem *next;
} ConfigItem;

// 配置管理器状态
typedef struct {
    ConfigItem *items;
    int item_count;
    char config_file_path[MAX_PATH_LENGTH];
    int modified; // 是否有未保存的修改
} ConfigManager;

// 配置管理器函数
int config_manager_init(ConfigManager *cm, const char *config_file_path);
void config_manager_cleanup(ConfigManager *cm);
int config_manager_load(ConfigManager *cm);
int config_manager_save(ConfigManager *cm);
int config_manager_reload(ConfigManager *cm);

// 配置项操作
ConfigItem *config_manager_get_item(ConfigManager *cm, const char *section, const char *key);
int config_manager_set_value(ConfigManager *cm, const char *section, const char *key, const char *value);
const char *config_manager_get_value(ConfigManager *cm, const char *section, const char *key);
int config_manager_reset_to_default(ConfigManager *cm, const char *section, const char *key);

// 搜索和过滤
ConfigItem *config_manager_find_items(ConfigManager *cm, const char *pattern);
ConfigItem *config_manager_get_items_in_section(ConfigManager *cm, const char *section);

// 验证函数
int config_manager_validate_item(ConfigManager *cm, ConfigItem *item);
int config_manager_validate_all(ConfigManager *cm);

// 预设配置
int config_manager_load_preset(ConfigManager *cm, const char *preset_name);
int config_manager_save_preset(ConfigManager *cm, const char *preset_name);
char **config_manager_get_preset_list(int *count);

// TUI 对话框函数
void show_config_manager_dialog(void);
void show_config_section_dialog(ConfigManager *cm, const char *section);
void show_config_edit_dialog(ConfigManager *cm, ConfigItem *item);
void show_config_search_dialog(ConfigManager *cm);
void show_config_presets_dialog(ConfigManager *cm);
void show_config_validation_dialog(ConfigManager *cm);

// 配置项创建和销毁
ConfigItem *config_item_create(const char *section, const char *key, const char *value,
                              const char *default_value, const char *description, ConfigType type);
void config_item_destroy(ConfigItem *item);

// 工具函数
const char *config_type_to_string(ConfigType type);
ConfigType string_to_config_type(const char *str);

#endif