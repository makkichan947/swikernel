#ifndef SWIKERNEL_H
#define SWIKERNEL_H

#include "common_defs.h"

// 运行模式
typedef enum {
    MODE_TUI,
    MODE_LIST_KERNELS,
    MODE_INSTALL_KERNEL,
    MODE_HELP,
    MODE_INVALID
} RunMode;

// 配置结构
typedef struct {
    char log_file[256];
    int log_level;
    int backup_enabled;
    int auto_dependencies;
    int parallel_compilation;
    char install_kernel[128];
    char default_source_dir[256];
} SwikernelConfig;

// 内核信息结构
typedef struct KernelInfo {
    char name[128];
    char version[64];
    char arch[32];
    int installed;
    int is_running;
    char source_path[256];
    struct KernelInfo *next;
} KernelInfo;

// 全局函数声明
int load_config(SwikernelConfig *config);git add README.md
void set_default_config(SwikernelConfig *config);
int start_tui_interface(void);
int list_available_kernels(void);
int install_kernel_cli(const char *kernel_name);
KernelInfo *scan_installed_kernels(void);
KernelInfo *get_available_kernels(void);
void free_kernel_list(KernelInfo *list);

// 全局配置外部引用
extern SwikernelConfig g_config;

#endif // SWIKERNEL_H