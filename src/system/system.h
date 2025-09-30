// src/system/system.h
#ifndef SYSTEM_H
#define SYSTEM_H

#include <sys/types.h>

// 系统调用相关
extern int errno;
int fast_syscall_wrapper(int syscall_num, ...);

// 文件操作
int mkdir_p(const char *path);
int copy_file(const char *src, const char *dst);
int remove_directory(const char *path);
int verify_file_integrity(const char *path, const char *expected_hash);
int calculate_sha256(const unsigned char *data, size_t len, char *output);

// 进程管理
int execute_command(const char *command);
char* execute_command_capture(const char *command);
pid_t execute_background(const char *command);
int wait_for_process(pid_t pid, int timeout_seconds);

// 系统信息
typedef struct {
    unsigned long totalram;
    unsigned long freeram;
    unsigned short procs;
    char cpu_arch[32];
} SystemInfo;

int get_system_info(SystemInfo *info);

#endif // SYSTEM_H