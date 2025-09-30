#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "kernel.h"
#include "logger.h"
#include "error_handler.h"
#include "dependency_check.h"
#include "rolling_update.h"

// 从源码安装内核
int install_kernel_from_source(const char *source_path, const char *kernel_name) {
    log_message(LOG_INFO, "Installing kernel from source: %s -> %s", 
            source_path, kernel_name);
    
    // 检查源码目录是否存在
    if (access(source_path, F_OK) != 0) {
        log_message(LOG_ERROR, "Source path does not exist: %s", source_path);
        return -1;
    }
    
    // 检查依赖
    DependencyStatus deps = check_system_dependencies();
    if (deps.missing_required_count > 0) {
        log_message(LOG_ERROR, "Missing required dependencies");
        free_dependency_status(&deps);
        return -1;
    }
    free_dependency_status(&deps);
    
    // 备份当前配置
    if (!backup_system_config()) {
        log_message(LOG_ERROR, "Failed to backup system configuration");
        return -1;
    }
    
    int status = 0;
    pid_t pid;
    
    // 编译和安装步骤
    const char *steps[] = {
        "make mrproper",
        "make defconfig",
        "make -j$(nproc)",
        "sudo make modules_install", 
        "sudo make install",
        NULL
    };
    
    for (int i = 0; steps[i]; i++) {
        log_message(LOG_INFO, "Executing step %d: %s", i + 1, steps[i]);
        
        pid = fork();
        if (pid == 0) {
            // 子进程
            chdir(source_path);
            execl("/bin/sh", "sh", "-c", steps[i], NULL);
            exit(1); // execl 失败
        } else if (pid > 0) {
            // 父进程等待
            waitpid(pid, &status, 0);
            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                log_message(LOG_ERROR, "Step %d failed: %s", i + 1, steps[i]);
                execute_rollback();
                return -1;
            }
        } else {
            log_message(LOG_ERROR, "Fork failed for step %d", i + 1);
            execute_rollback();
            return -1;
        }
    }
    
    // 更新引导配置
    if (apply_rolling_updates() != 0) {
        log_message(LOG_ERROR, "Failed to apply rolling updates");
        execute_rollback();
        return -1;
    }
    
    // 记录成功安装
    log_message(LOG_INFO, "Kernel installed successfully: %s", kernel_name);
    
    // 清除回滚栈（安装成功）
    clear_rollback_stack();
    
    return 0;
}

// 备份系统配置
int backup_system_config(void) {
    char timestamp[64];
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm);
    
    char backup_dir[256];
    snprintf(backup_dir, sizeof(backup_dir), "/var/lib/swikernel/backups/%s", timestamp);
    
    // 创建备份目录
    if (mkdir_p(backup_dir) != 0) {
        log_message(LOG_ERROR, "Failed to create backup directory: %s", backup_dir);
        return 0;
    }
    
    // 备份 GRUB 配置
    char grub_backup[512];
    snprintf(grub_backup, sizeof(grub_backup), "cp /boot/grub/grub.cfg %s/", backup_dir);
    if (system(grub_backup) != 0) {
        log_message(LOG_WARNING, "Failed to backup GRUB configuration");
    }
    
    // 添加回滚步骤
    ConfigBackupData backup_data;
    snprintf(backup_data.backup_path, sizeof(backup_data.backup_path), "%s", backup_dir);
    snprintf(backup_data.original_path, sizeof(backup_data.original_path), "/boot");
    
    add_rollback_step(ACTION_RESTORE_CONFIG, &backup_data, sizeof(backup_data));
    
    return 1;
}