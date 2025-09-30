#ifndef ASM_SYSCALLS_H
#define ASM_SYSCALLS_H

#include <stddef.h>
#include "common_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// 系统调用编号 (x86_64)
#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_STAT 4
#define SYS_FSTAT 5
#define SYS_LSTAT 6
#define SYS_POLL 7
#define SYS_LSEEK 8
#define SYS_MMAP 9
#define SYS_MPROTECT 10
#define SYS_MUNMAP 11
#define SYS_BRK 12
#define SYS_IOCTL 16
#define SYS_PREAD64 17
#define SYS_PWRITE64 18
#define SYS_ACCESS 21
#define SYS_PIPE 22
#define SYS_SELECT 23
#define SYS_DUP 32
#define SYS_DUP2 33
#define SYS_GETPID 39
#define SYS_SOCKET 41
#define SYS_CONNECT 42
#define SYS_ACCEPT 43
#define SYS_SENDTO 44
#define SYS_RECVFROM 45
#define SYS_SENDMSG 46
#define SYS_RECVMSG 47
#define SYS_SHUTDOWN 48
#define SYS_BIND 49
#define SYS_LISTEN 50
#define SYS_GETSOCKNAME 51
#define SYS_GETPEERNAME 52
#define SYS_SETSOCKOPT 54
#define SYS_GETSOCKOPT 55
#define SYS_EXECVE 59
#define SYS_EXIT 60
#define SYS_WAIT4 61
#define SYS_KILL 62
#define SYS_UNAME 63
#define SYS_FCNTL 72
#define SYS_FSYNC 74
#define SYS_FTRUNCATE 77
#define SYS_GETDENTS 78
#define SYS_GETCWD 79
#define SYS_CHDIR 80
#define SYS_FCHDIR 81
#define SYS_RENAME 82
#define SYS_MKDIR 83
#define SYS_RMDIR 84
#define SYS_CREAT 85
#define SYS_LINK 86
#define SYS_UNLINK 87
#define SYS_SYMLINK 88
#define SYS_READLINK 89
#define SYS_CHMOD 90
#define SYS_FCHMOD 91
#define SYS_CHOWN 92
#define SYS_FCHOWN 93
#define SYS_LCHOWN 94
#define SYS_UMASK 95
#define SYS_GETTIMEOFDAY 96
#define SYS_GETRLIMIT 97
#define SYS_GETRUSAGE 98
#define SYS_SYSINFO 99

// 系统调用包装函数
extern int fast_syscall(int syscall_num, ...);
extern int fast_file_check(const char *path);
extern void fast_get_time(char *buffer);
extern void* syscall_execute(int syscall_num, void *arg1, void *arg2, void *arg3);
extern int get_system_info(void *info);

// 内联汇编系统调用 (x86_64)
static FORCE_INLINE long syscall0(long num) {
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static FORCE_INLINE long syscall1(long num, long arg1) {
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static FORCE_INLINE long syscall2(long num, long arg1, long arg2) {
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1), "S"(arg2)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static FORCE_INLINE long syscall3(long num, long arg1, long arg2, long arg3) {
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static FORCE_INLINE long syscall4(long num, long arg1, long arg2, long arg3, long arg4) {
    long ret;
    register long r10 asm("r10") = arg4;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static FORCE_INLINE long syscall5(long num, long arg1, long arg2, long arg3, long arg4, long arg5) {
    long ret;
    register long r10 asm("r10") = arg4;
    register long r8 asm("r8") = arg5;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static FORCE_INLINE long syscall6(long num, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6) {
    long ret;
    register long r10 asm("r10") = arg4;
    register long r8 asm("r8") = arg5;
    register long r9 asm("r9") = arg6;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "memory"
    );
    return ret;
}

// 常用系统调用的包装函数
static FORCE_INLINE int swk_open(const char *pathname, int flags, mode_t mode) {
    return syscall3(SYS_OPEN, (long)pathname, flags, mode);
}

static FORCE_INLINE int swk_close(int fd) {
    return syscall1(SYS_CLOSE, fd);
}

static FORCE_INLINE ssize_t swk_read(int fd, void *buf, size_t count) {
    return syscall3(SYS_READ, fd, (long)buf, count);
}

static FORCE_INLINE ssize_t swk_write(int fd, const void *buf, size_t count) {
    return syscall3(SYS_WRITE, fd, (long)buf, count);
}

static FORCE_INLINE int swk_stat(const char *path, struct stat *statbuf) {
    return syscall2(SYS_STAT, (long)path, (long)statbuf);
}

static FORCE_INLINE int swk_fstat(int fd, struct stat *statbuf) {
    return syscall2(SYS_FSTAT, fd, (long)statbuf);
}

static FORCE_INLINE off_t swk_lseek(int fd, off_t offset, int whence) {
    return syscall3(SYS_LSEEK, fd, offset, whence);
}

static FORCE_INLINE void* swk_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return (void*)syscall6(SYS_MMAP, (long)addr, length, prot, flags, fd, offset);
}

static FORCE_INLINE int swk_munmap(void *addr, size_t length) {
    return syscall2(SYS_MUNMAP, (long)addr, length);
}

static FORCE_INLINE int swk_ioctl(int fd, unsigned long request, void *argp) {
    return syscall3(SYS_IOCTL, fd, request, (long)argp);
}

static FORCE_INLINE int swk_access(const char *pathname, int mode) {
    return syscall2(SYS_ACCESS, (long)pathname, mode);
}

static FORCE_INLINE pid_t swk_getpid(void) {
    return syscall0(SYS_GETPID);
}

static FORCE_INLINE int swk_uname(struct utsname *buf) {
    return syscall1(SYS_UNAME, (long)buf);
}

static FORCE_INLINE int swk_getcwd(char *buf, size_t size) {
    return syscall2(SYS_GETCWD, (long)buf, size);
}

static FORCE_INLINE int swk_chdir(const char *path) {
    return syscall1(SYS_CHDIR, (long)path);
}

static FORCE_INLINE int swk_mkdir(const char *pathname, mode_t mode) {
    return syscall2(SYS_MKDIR, (long)pathname, mode);
}

static FORCE_INLINE int swk_rmdir(const char *pathname) {
    return syscall1(SYS_RMDIR, (long)pathname);
}

static FORCE_INLINE int swk_rename(const char *oldpath, const char *newpath) {
    return syscall2(SYS_RENAME, (long)oldpath, (long)newpath);
}

static FORCE_INLINE int swk_unlink(const char *pathname) {
    return syscall1(SYS_UNLINK, (long)pathname);
}

static FORCE_INLINE int swk_symlink(const char *target, const char *linkpath) {
    return syscall2(SYS_SYMLINK, (long)target, (long)linkpath);
}

static FORCE_INLINE int swk_gettimeofday(struct timeval *tv, struct timezone *tz) {
    return syscall2(SYS_GETTIMEOFDAY, (long)tv, (long)tz);
}

static FORCE_INLINE int swk_sysinfo(struct sysinfo *info) {
    return syscall1(SYS_SYSINFO, (long)info);
}

#ifdef __cplusplus
}
#endif

#endif // ASM_SYSCALLS_H