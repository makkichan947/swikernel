#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ftw.h>
#include "kernel.h"
#include "logger.h"

// 回滚数据结构
typedef struct RollbackOperation {
    int operation_type;
    char source[512];
    char destination[512];
    char data[1024];
    struct RollbackOperation *next;
} RollbackOperation;

static RollbackOperation *rollback_operations = NULL;

// 添加回滚操作
void add_rollback_operation(int type, const char *src, const char *dst, const char *data) {
    RollbackOperation *op = malloc(sizeof(RollbackOperation));
    if (!op) {
        log_message(LOG_ERROR, "Failed to allocate rollback operation");
        return;
    }
    
    op->operation_type = type;
    if (src) strncpy(op->source, src, sizeof(op->source) - 1);
    if (dst) strncpy(op->destination, dst, sizeof(op->destination) - 1);
    if (data) strncpy(op->data, data, sizeof(op->data) - 1);
    
    op->next = rollback_operations;
    rollback_operations = op;
    
    log_message(LOG_DEBUG, "Added rollback operation type %d", type);
}

// 执行内核安装回滚
int rollback_kernel_installation(const char *kernel_name) {
    log_message(LOG_INFO, "Rolling back kernel installation: %s", kernel_name);
    
    int success = 1;
    
    // 删除内核镜像
    char vmlinuz_path[256];
    snprintf(vmlinuz_path, sizeof(vmlinuz_path), "/boot/vmlinuz-%s", kernel_name);
    if (unlink(vmlinuz_path) != 0) {
        log_message(LOG_WARNING, "Could not remove %s", vmlinuz_path);
        success = 0;
    }
    
    // 删除 initrd
    char initrd_path[256];
    snprintf(initrd_path, sizeof(initrd_path), "/boot/initrd.img-%s", kernel_name);
    if (unlink(initrd_path) != 0) {
        log_message(LOG_WARNING, "Could not remove %s", initrd_path);
    }
    
    // 删除 System.map
    char system_map_path[256];
    snprintf(system_map_path, sizeof(system_map_path), "/boot/System.map-%s", kernel_name);
    if (unlink(system_map_path) != 0) {
        log_message(LOG_WARNING, "Could not remove %s", system_map_path);
    }
    
    // 删除配置
    char config_path[256];
    snprintf(config_path, sizeof(config_path), "/boot/config-%s", kernel_name);
    if (unlink(config_path) != 0) {
        log_message(LOG_WARNING, "Could not remove %s", config_path);
    }
    
    // 删除模块
    char modules_path[256];
    snprintf(modules_path, sizeof(modules_path), "/lib/modules/%s", kernel_name);
    if (system_rmrf(modules_path) != 0) {
        log_message(LOG_WARNING, "Could not remove modules directory %s", modules_path);
    }
    
    // 更新 GRUB
    if (system("update-grub") != 0) {
        log_message(LOG_WARNING, "Failed to update GRUB after rollback");
    }
    
    if (success) {
        log_message(LOG_INFO, "Kernel rollback completed: %s", kernel_name);
    } else {
        log_message(LOG_WARNING, "Kernel rollback completed with warnings: %s", kernel_name);
    }
    
    return success ? 0 : -1;
}

// 递归删除目录（用于模块清理）
int system_rmrf(const char *path) {
    char command[512];
    snprintf(command, sizeof(command), "rm -rf \"%s\"", path);
    return system(command);
}