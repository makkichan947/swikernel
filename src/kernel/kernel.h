#ifndef KERNEL_H
#define KERNEL_H

#include "swikernel.h"

// 内核操作函数
KernelInfo *scan_installed_kernels(void);
int get_current_kernel(char *buffer, size_t size);
void free_kernel_list(KernelInfo *list);
int list_available_kernels(void);

// 内核安装
int install_kernel_from_source(const char *source_path, const char *kernel_name);
int install_kernel_from_repo(const char *kernel_name);
int backup_system_config(void);

// 回滚机制
int rollback_kernel_installation(const char *kernel_name);
void add_rollback_operation(int type, const char *src, const char *dst, const char *data);

// 依赖检查
DependencyStatus check_system_dependencies(void);
void free_dependency_status(DependencyStatus *status);

// 系统工具
int system_rmrf(const char *path);
int mkdir_p(const char *path);

#endif // KERNEL_H