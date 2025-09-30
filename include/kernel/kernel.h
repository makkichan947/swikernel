#ifndef KERNEL_H
#define KERNEL_H

#include "../swikernel.h"

// 内核操作函数
KernelInfo *scan_installed_kernels(void);
int get_current_kernel(char *buffer, size_t size);
void free_kernel_list(KernelInfo *list);
int list_available_kernels(void);
KernelInfo *find_kernel_by_name(const char *name);

// 内核安装
int install_kernel_from_source(const char *source_path, const char *kernel_name);
int install_kernel_from_repo(const char *kernel_name);
int backup_system_config(void);
int build_kernel(const char *source_path, const char *output_path, ProgressCallback callback, void *user_data);
int install_built_kernel(const char *build_path, const char *kernel_name);

// 回滚机制
int rollback_kernel_installation(const char *kernel_name);
void add_rollback_operation(int type, const char *src, const char *dst, const char *data);

// 依赖检查
DependencyStatus check_system_dependencies(void);
void free_dependency_status(DependencyStatus *status);

// 验证函数
int validate_kernel_name(const char *name);
int validate_version_string(const char *version);

// 系统工具
int system_rmrf(const char *path);
int mkdir_p(const char *path);
char* get_file_checksum(const char *path);

// 备份和恢复
int create_backup(const char *source, const char *backup_dir);
int restore_backup(const char *backup_dir, const char *target);

#endif