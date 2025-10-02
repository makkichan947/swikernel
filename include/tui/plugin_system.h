#ifndef PLUGIN_SYSTEM_H
#define PLUGIN_SYSTEM_H

#include "../common_defs.h"

// 插件类型
typedef enum {
    PLUGIN_TYPE_KERNEL_MANAGER,
    PLUGIN_TYPE_FILE_MANAGER,
    PLUGIN_TYPE_LOG_VIEWER,
    PLUGIN_TYPE_CONFIG_MANAGER,
    PLUGIN_TYPE_SYSTEM_MONITOR,
    PLUGIN_TYPE_THEME_MANAGER,
    PLUGIN_TYPE_CUSTOM
} PluginType;

// 插件信息结构
typedef struct PluginInfo {
    char name[128];
    char version[32];
    char description[256];
    char author[128];
    char file_path[MAX_PATH_LENGTH];
    PluginType type;
    void *handle; // 动态库句柄
    int enabled;
    int loaded;
    struct PluginInfo *next;
} PluginInfo;

// 插件接口函数类型
typedef int (*PluginInitFunc)(void);
typedef void (*PluginCleanupFunc)(void);
typedef const char* (*PluginGetNameFunc)(void);
typedef const char* (*PluginGetVersionFunc)(void);
typedef const char* (*PluginGetDescriptionFunc)(void);
typedef PluginType (*PluginGetTypeFunc)(void);
typedef int (*PluginExecuteFunc)(void *data);

// 插件系统状态
typedef struct {
    PluginInfo *plugins;
    int plugin_count;
    int enabled_count;
    char plugin_dir[MAX_PATH_LENGTH];
    int auto_load;
} PluginSystem;

// 插件系统函数
int plugin_system_init(PluginSystem *ps, const char *plugin_dir);
void plugin_system_cleanup(PluginSystem *ps);
int plugin_system_load_plugin(PluginSystem *ps, const char *plugin_path);
int plugin_system_unload_plugin(PluginSystem *ps, const char *plugin_name);
int plugin_system_enable_plugin(PluginSystem *ps, const char *plugin_name);
int plugin_system_disable_plugin(PluginSystem *ps, const char *plugin_name);
int plugin_system_scan_plugins(PluginSystem *ps);

// 插件查询函数
PluginInfo *plugin_system_get_plugin(PluginSystem *ps, const char *name);
PluginInfo *plugin_system_get_plugins_by_type(PluginSystem *ps, PluginType type);
PluginInfo *plugin_system_get_enabled_plugins(PluginSystem *ps);

// 插件执行函数
int plugin_system_execute_plugin(PluginSystem *ps, const char *name, void *data);
int plugin_system_execute_plugins_by_type(PluginSystem *ps, PluginType type, void *data);

// 插件管理函数
int plugin_system_install_plugin(PluginSystem *ps, const char *plugin_file);
int plugin_system_remove_plugin(PluginSystem *ps, const char *plugin_name);
int plugin_system_update_plugin(PluginSystem *ps, const char *plugin_name);

// 插件验证函数
int plugin_system_validate_plugin(PluginSystem *ps, const char *plugin_path);
int plugin_system_verify_signature(PluginSystem *ps, const char *plugin_path);

// TUI 对话框函数
void show_plugin_manager_dialog(void);
void show_plugin_info_dialog(PluginInfo *plugin);
void show_plugin_install_dialog(PluginSystem *ps);
void show_plugin_list_dialog(PluginSystem *ps);

// 插件模板和工具
int plugin_system_create_template(PluginSystem *ps, const char *name, PluginType type, const char *template_path);
const char *plugin_system_get_template(PluginType type);

// 插件配置
int plugin_system_save_config(PluginSystem *ps);
int plugin_system_load_config(PluginSystem *ps);

#endif