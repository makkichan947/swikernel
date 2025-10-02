#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include "plugin_system.h"
#include "logger.h"

// 初始化插件系统
int plugin_system_init(PluginSystem *ps, const char *plugin_dir) {
    if (!ps || !plugin_dir) {
        return SWK_ERROR_INVALID_PARAM;
    }

    memset(ps, 0, sizeof(PluginSystem));
    strncpy(ps->plugin_dir, plugin_dir, sizeof(ps->plugin_dir) - 1);
    ps->auto_load = 1;

    // 创建插件目录（如果不存在）
    struct stat st = {0};
    if (stat(plugin_dir, &st) == -1) {
        if (mkdir(plugin_dir, 0755) != 0) {
            log_message(LOG_ERROR, "Failed to create plugin directory: %s", plugin_dir);
            return SWK_ERROR_SYSTEM_CALL;
        }
    }

    // 扫描现有插件
    return plugin_system_scan_plugins(ps);
}

// 清理插件系统
void plugin_system_cleanup(PluginSystem *ps) {
    if (!ps) return;

    // 卸载所有插件
    PluginInfo *current = ps->plugins;
    while (current) {
        plugin_system_unload_plugin(ps, current->name);
        current = current->next;
    }

    ps->plugins = NULL;
    ps->plugin_count = 0;
    ps->enabled_count = 0;
}

// 扫描插件目录
int plugin_system_scan_plugins(PluginSystem *ps) {
    if (!ps) {
        return SWK_ERROR_INVALID_PARAM;
    }

    DIR *dir;
    struct dirent *entry;

    dir = opendir(ps->plugin_dir);
    if (!dir) {
        log_message(LOG_ERROR, "Cannot open plugin directory: %s", ps->plugin_dir);
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    while ((entry = readdir(dir)) != NULL) {
        // 只处理 .so 文件
        if (strstr(entry->d_name, ".so") == NULL) {
            continue;
        }

        char plugin_path[MAX_PATH_LENGTH];
        snprintf(plugin_path, sizeof(plugin_path), "%s/%s", ps->plugin_dir, entry->d_name);

        // 验证插件文件
        if (plugin_system_validate_plugin(ps, plugin_path) == SWK_SUCCESS) {
            plugin_system_load_plugin(ps, plugin_path);
        }
    }

    closedir(dir);
    log_message(LOG_DEBUG, "Scanned %d plugins in %s", ps->plugin_count, ps->plugin_dir);
    return SWK_SUCCESS;
}

// 加载插件
int plugin_system_load_plugin(PluginSystem *ps, const char *plugin_path) {
    if (!ps || !plugin_path) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 打开动态库
    void *handle = dlopen(plugin_path, RTLD_LAZY);
    if (!handle) {
        log_message(LOG_ERROR, "Failed to load plugin %s: %s", plugin_path, dlerror());
        return SWK_ERROR_SYSTEM_CALL;
    }

    // 获取插件信息函数
    PluginGetNameFunc get_name = (PluginGetNameFunc)dlsym(handle, "plugin_get_name");
    PluginGetVersionFunc get_version = (PluginGetVersionFunc)dlsym(handle, "plugin_get_version");
    PluginGetDescriptionFunc get_description = (PluginGetDescriptionFunc)dlsym(handle, "plugin_get_description");
    PluginGetTypeFunc get_type = (PluginGetTypeFunc)dlsym(handle, "plugin_get_type");

    if (!get_name || !get_version || !get_description || !get_type) {
        log_message(LOG_ERROR, "Invalid plugin %s: missing required functions", plugin_path);
        dlclose(handle);
        return SWK_ERROR_INVALID_PARAM;
    }

    // 创建插件信息
    PluginInfo *plugin = malloc(sizeof(PluginInfo));
    if (!plugin) {
        dlclose(handle);
        return SWK_ERROR_OUT_OF_MEMORY;
    }

    memset(plugin, 0, sizeof(PluginInfo));

    // 获取插件信息
    strncpy(plugin->name, get_name(), sizeof(plugin->name) - 1);
    strncpy(plugin->version, get_version(), sizeof(plugin->version) - 1);
    strncpy(plugin->description, get_description(), sizeof(plugin->description) - 1);
    strncpy(plugin->file_path, plugin_path, sizeof(plugin->file_path) - 1);
    plugin->type = get_type();
    plugin->handle = handle;
    plugin->enabled = 1;
    plugin->loaded = 1;

    // 添加到插件列表
    plugin->next = ps->plugins;
    ps->plugins = plugin;
    ps->plugin_count++;

    if (plugin->enabled) {
        ps->enabled_count++;
    }

    log_message(LOG_INFO, "Loaded plugin: %s v%s (%s)",
               plugin->name, plugin->version, plugin->description);
    return SWK_SUCCESS;
}

// 卸载插件
int plugin_system_unload_plugin(PluginSystem *ps, const char *plugin_name) {
    if (!ps || !plugin_name) {
        return SWK_ERROR_INVALID_PARAM;
    }

    PluginInfo *current = ps->plugins;
    PluginInfo *prev = NULL;

    while (current) {
        if (strcmp(current->name, plugin_name) == 0) {
            // 调用插件清理函数
            if (current->loaded && current->handle) {
                PluginCleanupFunc cleanup = (PluginCleanupFunc)dlsym(current->handle, "plugin_cleanup");
                if (cleanup) {
                    cleanup();
                }

                dlclose(current->handle);
            }

            // 从链表中移除
            if (prev) {
                prev->next = current->next;
            } else {
                ps->plugins = current->next;
            }

            if (current->enabled) {
                ps->enabled_count--;
            }

            ps->plugin_count--;
            free(current);

            log_message(LOG_INFO, "Unloaded plugin: %s", plugin_name);
            return SWK_SUCCESS;
        }

        prev = current;
        current = current->next;
    }

    return SWK_ERROR_FILE_NOT_FOUND;
}

// 启用插件
int plugin_system_enable_plugin(PluginSystem *ps, const char *plugin_name) {
    if (!ps || !plugin_name) {
        return SWK_ERROR_INVALID_PARAM;
    }

    PluginInfo *plugin = plugin_system_get_plugin(ps, plugin_name);
    if (!plugin) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    if (!plugin->enabled) {
        plugin->enabled = 1;
        ps->enabled_count++;
        log_message(LOG_INFO, "Enabled plugin: %s", plugin_name);
    }

    return SWK_SUCCESS;
}

// 禁用插件
int plugin_system_disable_plugin(PluginSystem *ps, const char *plugin_name) {
    if (!ps || !plugin_name) {
        return SWK_ERROR_INVALID_PARAM;
    }

    PluginInfo *plugin = plugin_system_get_plugin(ps, plugin_name);
    if (!plugin) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    if (plugin->enabled) {
        plugin->enabled = 0;
        ps->enabled_count--;
        log_message(LOG_INFO, "Disabled plugin: %s", plugin_name);
    }

    return SWK_SUCCESS;
}

// 获取插件信息
PluginInfo *plugin_system_get_plugin(PluginSystem *ps, const char *name) {
    if (!ps || !name) return NULL;

    PluginInfo *current = ps->plugins;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

// 获取指定类型的插件
PluginInfo *plugin_system_get_plugins_by_type(PluginSystem *ps, PluginType type) {
    if (!ps) return NULL;

    PluginInfo *result = NULL;
    PluginInfo *tail = NULL;
    PluginInfo *current = ps->plugins;

    while (current) {
        if (current->type == type && current->enabled) {
            PluginInfo *plugin = malloc(sizeof(PluginInfo));
            if (plugin) {
                memcpy(plugin, current, sizeof(PluginInfo));
                plugin->next = NULL;

                if (!result) {
                    result = plugin;
                    tail = plugin;
                } else {
                    tail->next = plugin;
                    tail = plugin;
                }
            }
        }
        current = current->next;
    }

    return result;
}

// 获取启用的插件
PluginInfo *plugin_system_get_enabled_plugins(PluginSystem *ps) {
    if (!ps) return NULL;

    PluginInfo *result = NULL;
    PluginInfo *tail = NULL;
    PluginInfo *current = ps->plugins;

    while (current) {
        if (current->enabled) {
            PluginInfo *plugin = malloc(sizeof(PluginInfo));
            if (plugin) {
                memcpy(plugin, current, sizeof(PluginInfo));
                plugin->next = NULL;

                if (!result) {
                    result = plugin;
                    tail = plugin;
                } else {
                    tail->next = plugin;
                    tail = plugin;
                }
            }
        }
        current = current->next;
    }

    return result;
}

// 执行插件
int plugin_system_execute_plugin(PluginSystem *ps, const char *name, void *data) {
    if (!ps || !name) {
        return SWK_ERROR_INVALID_PARAM;
    }

    PluginInfo *plugin = plugin_system_get_plugin(ps, name);
    if (!plugin || !plugin->enabled || !plugin->handle) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    PluginExecuteFunc execute = (PluginExecuteFunc)dlsym(plugin->handle, "plugin_execute");
    if (!execute) {
        log_message(LOG_ERROR, "Plugin %s does not have execute function", name);
        return SWK_ERROR_INVALID_PARAM;
    }

    log_message(LOG_DEBUG, "Executing plugin: %s", name);
    return execute(data);
}

// 执行指定类型的插件
int plugin_system_execute_plugins_by_type(PluginSystem *ps, PluginType type, void *data) {
    if (!ps) {
        return SWK_ERROR_INVALID_PARAM;
    }

    PluginInfo *plugins = plugin_system_get_plugins_by_type(ps, type);
    if (!plugins) {
        return SWK_SUCCESS; // 没有插件可执行
    }

    int result = SWK_SUCCESS;
    PluginInfo *current = plugins;

    while (current) {
        int ret = plugin_system_execute_plugin(ps, current->name, data);
        if (ret != SWK_SUCCESS) {
            result = ret;
        }
        current = current->next;
    }

    // 清理临时列表
    while (plugins) {
        PluginInfo *next = plugins->next;
        free(plugins);
        plugins = next;
    }

    return result;
}

// 验证插件文件
int plugin_system_validate_plugin(PluginSystem *ps, const char *plugin_path) {
    if (!ps || !plugin_path) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 检查文件是否存在
    struct stat st;
    if (stat(plugin_path, &st) != 0) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    // 检查是否为常规文件
    if (!S_ISREG(st.st_mode)) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 检查文件扩展名
    if (strstr(plugin_path, ".so") == NULL) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 尝试打开动态库进行基本验证
    void *handle = dlopen(plugin_path, RTLD_LAZY);
    if (!handle) {
        log_message(LOG_WARNING, "Invalid plugin file %s: %s", plugin_path, dlerror());
        return SWK_ERROR_INVALID_PARAM;
    }

    // 检查必需的函数
    if (!dlsym(handle, "plugin_get_name") ||
        !dlsym(handle, "plugin_get_version") ||
        !dlsym(handle, "plugin_get_description") ||
        !dlsym(handle, "plugin_get_type")) {
        dlclose(handle);
        return SWK_ERROR_INVALID_PARAM;
    }

    dlclose(handle);
    return SWK_SUCCESS;
}

// 安装插件
int plugin_system_install_plugin(PluginSystem *ps, const char *plugin_file) {
    if (!ps || !plugin_file) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 验证插件文件
    if (plugin_system_validate_plugin(ps, plugin_file) != SWK_SUCCESS) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 复制到插件目录
    char dest_path[MAX_PATH_LENGTH];
    char *file_name = strrchr(plugin_file, '/');
    if (file_name) {
        file_name++; // 跳过 '/'
    } else {
        file_name = (char*)plugin_file;
    }

    snprintf(dest_path, sizeof(dest_path), "%s/%s", ps->plugin_dir, file_name);

    char command[MAX_PATH_LENGTH * 2];
    snprintf(command, sizeof(command), "cp \"%s\" \"%s\"", plugin_file, dest_path);

    if (system(command) != 0) {
        log_message(LOG_ERROR, "Failed to install plugin %s", plugin_file);
        return SWK_ERROR_SYSTEM_CALL;
    }

    // 加载新安装的插件
    return plugin_system_load_plugin(ps, dest_path);
}

// 移除插件
int plugin_system_remove_plugin(PluginSystem *ps, const char *plugin_name) {
    if (!ps || !plugin_name) {
        return SWK_ERROR_INVALID_PARAM;
    }

    PluginInfo *plugin = plugin_system_get_plugin(ps, plugin_name);
    if (!plugin) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    // 卸载插件
    int result = plugin_system_unload_plugin(ps, plugin_name);

    if (result == SWK_SUCCESS) {
        // 删除插件文件
        if (unlink(plugin->file_path) != 0) {
            log_message(LOG_WARNING, "Failed to remove plugin file: %s", plugin->file_path);
        } else {
            log_message(LOG_INFO, "Removed plugin file: %s", plugin->file_path);
        }
    }

    return result;
}