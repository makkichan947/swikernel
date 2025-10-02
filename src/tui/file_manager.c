#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include "file_manager.h"
#include "logger.h"
#include "common_defs.h"

// 初始化文件管理器
int file_manager_init(FileManager *fm, const char *start_path) {
    if (!fm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    memset(fm, 0, sizeof(FileManager));
    fm->show_hidden = 0;
    fm->current_selection = 0;
    fm->scroll_offset = 0;

    if (start_path && strlen(start_path) > 0) {
        strncpy(fm->current_path, start_path, sizeof(fm->current_path) - 1);
    } else {
        getcwd(fm->current_path, sizeof(fm->current_path));
    }

    return file_manager_refresh(fm);
}

// 清理文件管理器
void file_manager_cleanup(FileManager *fm) {
    if (!fm) return;

    // 释放文件列表
    if (fm->files) {
        FileInfo *current = fm->files;
        while (current) {
            FileInfo *next = current->next;
            free(current);
            current = next;
        }
        fm->files = NULL;
    }

    fm->file_count = 0;
}

// 刷新文件列表
int file_manager_refresh(FileManager *fm) {
    if (!fm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 清理现有文件列表
    file_manager_cleanup(fm);

    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char full_path[MAX_PATH_LENGTH];
    FileInfo *tail = NULL;

    dir = opendir(fm->current_path);
    if (!dir) {
        log_message(LOG_ERROR, "Cannot open directory: %s", fm->current_path);
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    while ((entry = readdir(dir)) != NULL) {
        // 跳过隐藏文件（除非启用了显示隐藏文件）
        if (entry->d_name[0] == '.' && !fm->show_hidden) {
            continue;
        }

        // 应用过滤器
        if (strlen(fm->filter_pattern) > 0) {
            if (strstr(entry->d_name, fm->filter_pattern) == NULL) {
                continue;
            }
        }

        // 获取完整路径和文件信息
        snprintf(full_path, sizeof(full_path), "%s/%s", fm->current_path, entry->d_name);

        if (stat(full_path, &statbuf) == -1) {
            continue;
        }

        // 创建文件信息节点
        FileInfo *info = malloc(sizeof(FileInfo));
        if (!info) {
            log_message(LOG_ERROR, "Failed to allocate memory for file info");
            continue;
        }

        memset(info, 0, sizeof(FileInfo));
        strncpy(info->name, entry->d_name, sizeof(info->name) - 1);
        strncpy(info->path, full_path, sizeof(info->path) - 1);

        // 文件类型
        info->is_directory = S_ISDIR(statbuf.st_mode);

        // 文件大小
        if (info->is_directory) {
            strcpy(info->size, "<DIR>");
        } else {
            snprintf(info->size, sizeof(info->size), "%lld", (long long)statbuf.st_size);
        }

        // 权限
        snprintf(info->permissions, sizeof(info->permissions),
                "%c%c%c%c%c%c%c%c%c",
                S_ISDIR(statbuf.st_mode) ? 'd' : '-',
                statbuf.st_mode & S_IRUSR ? 'r' : '-',
                statbuf.st_mode & S_IWUSR ? 'w' : '-',
                statbuf.st_mode & S_IXUSR ? 'x' : '-',
                statbuf.st_mode & S_IRGRP ? 'r' : '-',
                statbuf.st_mode & S_IWGRP ? 'w' : '-',
                statbuf.st_mode & S_IXGRP ? 'x' : '-',
                statbuf.st_mode & S_IROTH ? 'r' : '-',
                statbuf.st_mode & S_IWOTH ? 'w' : '-',
                statbuf.st_mode & S_IXOTH ? 'x' : '-');

        // 所有者和组
        struct passwd *pw = getpwuid(statbuf.st_uid);
        struct group *gr = getgrgid(statbuf.st_gid);

        if (pw) {
            strncpy(info->owner, pw->pw_name, sizeof(info->owner) - 1);
        } else {
            snprintf(info->owner, sizeof(info->owner), "%d", statbuf.st_uid);
        }

        if (gr) {
            strncpy(info->group, gr->gr_name, sizeof(info->group) - 1);
        } else {
            snprintf(info->group, sizeof(info->group), "%d", statbuf.st_gid);
        }

        // 修改时间
        struct tm *tm = localtime(&statbuf.st_mtime);
        strftime(info->modified_time, sizeof(info->modified_time), "%Y-%m-%d %H:%M", tm);

        // 添加到链表
        if (!fm->files) {
            fm->files = info;
            tail = info;
        } else {
            tail->next = info;
            tail = info;
        }

        fm->file_count++;
    }

    closedir(dir);
    log_message(LOG_DEBUG, "Refreshed file list: %d files in %s", fm->file_count, fm->current_path);
    return SWK_SUCCESS;
}

// 切换目录
int file_manager_change_directory(FileManager *fm, const char *path) {
    if (!fm || !path) {
        return SWK_ERROR_INVALID_PARAM;
    }

    char new_path[MAX_PATH_LENGTH];
    struct stat statbuf;

    // 处理相对路径
    if (path[0] != '/') {
        snprintf(new_path, sizeof(new_path), "%s/%s", fm->current_path, path);
    } else {
        strncpy(new_path, path, sizeof(new_path) - 1);
    }

    // 标准化路径
    char resolved_path[MAX_PATH_LENGTH];
    if (realpath(new_path, resolved_path) == NULL) {
        log_message(LOG_ERROR, "Invalid path: %s", new_path);
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    // 检查是否为目录
    if (stat(resolved_path, &statbuf) == -1 || !S_ISDIR(statbuf.st_mode)) {
        log_message(LOG_ERROR, "Not a directory: %s", resolved_path);
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    strncpy(fm->current_path, resolved_path, sizeof(fm->current_path) - 1);
    return file_manager_refresh(fm);
}

// 向上导航
int file_manager_navigate_up(FileManager *fm) {
    if (!fm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    char parent_path[MAX_PATH_LENGTH];
    strncpy(parent_path, fm->current_path, sizeof(parent_path) - 1);

    char *last_slash = strrchr(parent_path, '/');
    if (last_slash) {
        *last_slash = '\0';
        if (strlen(parent_path) == 0) {
            strcpy(parent_path, "/");
        }
        return file_manager_change_directory(fm, parent_path);
    }

    return SWK_ERROR_INVALID_PARAM;
}

// 复制文件
FileOperationResult file_manager_copy(FileManager *fm, const char *source, const char *destination) {
    FileOperationResult result;
    memset(&result, 0, sizeof(result));

    if (!fm || !source || !destination) {
        result.code = SWK_ERROR_INVALID_PARAM;
        strcpy(result.message, "Invalid parameters");
        return result;
    }

    char command[MAX_PATH_LENGTH * 2];
    snprintf(command, sizeof(command), "cp -r \"%s\" \"%s\"", source, destination);

    if (system(command) == 0) {
        result.code = SWK_SUCCESS;
        strcpy(result.message, "File copied successfully");
        strncpy(result.source_path, source, sizeof(result.source_path) - 1);
        strncpy(result.target_path, destination, sizeof(result.target_path) - 1);
        log_message(LOG_INFO, "Copied %s to %s", source, destination);
    } else {
        result.code = SWK_ERROR_SYSTEM_CALL;
        snprintf(result.message, sizeof(result.message), "Failed to copy file: %s", strerror(errno));
        log_message(LOG_ERROR, "Failed to copy %s to %s: %s", source, destination, strerror(errno));
    }

    return result;
}

// 删除文件
FileOperationResult file_manager_delete(FileManager *fm, const char *path) {
    FileOperationResult result;
    memset(&result, 0, sizeof(result));

    if (!fm || !path) {
        result.code = SWK_ERROR_INVALID_PARAM;
        strcpy(result.message, "Invalid parameters");
        return result;
    }

    char command[MAX_PATH_LENGTH * 2];
    snprintf(command, sizeof(command), "rm -rf \"%s\"", path);

    if (system(command) == 0) {
        result.code = SWK_SUCCESS;
        strcpy(result.message, "File deleted successfully");
        strncpy(result.source_path, path, sizeof(result.source_path) - 1);
        log_message(LOG_INFO, "Deleted %s", path);
    } else {
        result.code = SWK_ERROR_SYSTEM_CALL;
        snprintf(result.message, sizeof(result.message), "Failed to delete file: %s", strerror(errno));
        log_message(LOG_ERROR, "Failed to delete %s: %s", path, strerror(errno));
    }

    return result;
}

// 创建目录
FileOperationResult file_manager_mkdir(FileManager *fm, const char *name) {
    FileOperationResult result;
    memset(&result, 0, sizeof(result));

    if (!fm || !name) {
        result.code = SWK_ERROR_INVALID_PARAM;
        strcpy(result.message, "Invalid parameters");
        return result;
    }

    char new_dir[MAX_PATH_LENGTH];
    snprintf(new_dir, sizeof(new_dir), "%s/%s", fm->current_path, name);

    if (mkdir(new_dir, 0755) == 0) {
        result.code = SWK_SUCCESS;
        strcpy(result.message, "Directory created successfully");
        strncpy(result.target_path, new_dir, sizeof(result.target_path) - 1);
        log_message(LOG_INFO, "Created directory %s", new_dir);
    } else {
        result.code = SWK_ERROR_SYSTEM_CALL;
        snprintf(result.message, sizeof(result.message), "Failed to create directory: %s", strerror(errno));
        log_message(LOG_ERROR, "Failed to create directory %s: %s", new_dir, strerror(errno));
    }

    return result;
}

// 重命名文件
FileOperationResult file_manager_rename(FileManager *fm, const char *old_name, const char *new_name) {
    FileOperationResult result;
    memset(&result, 0, sizeof(result));

    if (!fm || !old_name || !new_name) {
        result.code = SWK_ERROR_INVALID_PARAM;
        strcpy(result.message, "Invalid parameters");
        return result;
    }

    char old_path[MAX_PATH_LENGTH];
    char new_path[MAX_PATH_LENGTH];

    snprintf(old_path, sizeof(old_path), "%s/%s", fm->current_path, old_name);
    snprintf(new_path, sizeof(new_path), "%s/%s", fm->current_path, new_name);

    if (rename(old_path, new_path) == 0) {
        result.code = SWK_SUCCESS;
        strcpy(result.message, "File renamed successfully");
        strncpy(result.source_path, old_path, sizeof(result.source_path) - 1);
        strncpy(result.target_path, new_path, sizeof(result.target_path) - 1);
        log_message(LOG_INFO, "Renamed %s to %s", old_path, new_path);
    } else {
        result.code = SWK_ERROR_SYSTEM_CALL;
        snprintf(result.message, sizeof(result.message), "Failed to rename file: %s", strerror(errno));
        log_message(LOG_ERROR, "Failed to rename %s to %s: %s", old_path, new_path, strerror(errno));
    }

    return result;
}

// 设置过滤器
void file_manager_set_filter(FileManager *fm, const char *pattern) {
    if (!fm || !pattern) return;

    strncpy(fm->filter_pattern, pattern, sizeof(fm->filter_pattern) - 1);
    file_manager_refresh(fm);
}

// 切换隐藏文件显示
void file_manager_toggle_hidden_files(FileManager *fm) {
    if (!fm) return;

    fm->show_hidden = !fm->show_hidden;
    file_manager_refresh(fm);
}

// 获取当前路径
const char *file_manager_get_current_path(FileManager *fm) {
    if (!fm) return NULL;
    return fm->current_path;
}

// 获取文件数量
int file_manager_get_file_count(FileManager *fm) {
    if (!fm) return 0;
    return fm->file_count;
}