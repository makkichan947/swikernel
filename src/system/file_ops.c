// src/system/file_ops.c
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include "system.h"
#include "logger.h"

// 创建目录（递归）
int mkdir_p(const char *path) {
    char tmp[1024];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, S_IRWXU) != 0 && errno != EEXIST) {
                log_message(LOG_ERROR, "Failed to create directory: %s", tmp);
                return -1;
            }
            *p = '/';
        }
    }
    
    if (mkdir(tmp, S_IRWXU) != 0 && errno != EEXIST) {
        log_message(LOG_ERROR, "Failed to create directory: %s", tmp);
        return -1;
    }

    log_message(LOG_DEBUG, "Created directory: %s", path);
    return 0;
}

// 复制文件
int copy_file(const char *src, const char *dst) {
    int src_fd, dst_fd;
    struct stat st;
    char buffer[8192];
    ssize_t bytes_read, bytes_written;

    // 打开源文件
    src_fd = open(src, O_RDONLY);
    if (src_fd < 0) {
        log_message(LOG_ERROR, "Failed to open source file: %s", src);
        return -1;
    }

    // 获取文件信息
    if (fstat(src_fd, &st) != 0) {
        log_message(LOG_ERROR, "Failed to stat source file: %s", src);
        close(src_fd);
        return -1;
    }

    // 创建目标文件
    dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    if (dst_fd < 0) {
        log_message(LOG_ERROR, "Failed to create destination file: %s", dst);
        close(src_fd);
        return -1;
    }

    // 复制数据
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dst_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            log_message(LOG_ERROR, "Write error to file: %s", dst);
            close(src_fd);
            close(dst_fd);
            return -1;
        }
    }

    close(src_fd);
    close(dst_fd);
    
    log_message(LOG_DEBUG, "Copied file: %s -> %s", src, dst);
    return 0;
}

// 递归删除目录
int remove_directory(const char *path) {
    DIR *dir;
    struct dirent *entry;
    char full_path[1024];
    struct stat st;

    dir = opendir(path);
    if (!dir) {
        log_message(LOG_ERROR, "Cannot open directory: %s", path);
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (lstat(full_path, &st) != 0) {
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            remove_directory(full_path);
        } else {
            unlink(full_path);
        }
    }

    closedir(dir);
    rmdir(path);
    
    log_message(LOG_DEBUG, "Removed directory: %s", path);
    return 0;
}

// 文件完整性校验
int verify_file_integrity(const char *path, const char *expected_hash) {
    int fd;
    struct stat st;
    unsigned char *data;
    char actual_hash[65];
    
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    
    if (fstat(fd, &st) != 0) {
        close(fd);
        return -1;
    }
    
    data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return -1;
    }
    
    // 计算SHA-256哈希（简化版）
    calculate_sha256(data, st.st_size, actual_hash);
    
    munmap(data, st.st_size);
    close(fd);
    
    if (strcmp(actual_hash, expected_hash) == 0) {
        log_message(LOG_DEBUG, "File integrity verified: %s", path);
        return 0;
    } else {
        log_message(LOG_ERROR, "File integrity check failed: %s", path);
        return -1;
    }
}