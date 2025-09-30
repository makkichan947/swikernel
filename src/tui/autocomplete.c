#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include "autocomplete.h"
#include "logger.h"

// 路径自动补全函数
char** path_autocomplete(const char *partial_path, int *match_count) {
    static char *matches[MAX_MATCHES];
    *match_count = 0;
    
    if (!partial_path) return NULL;
    
    char dir_path[1024];
    char file_prefix[256];
    const char *last_slash = strrchr(partial_path, '/');
    
    if (last_slash) {
        size_t dir_len = last_slash - partial_path + 1;
        strncpy(dir_path, partial_path, dir_len);
        dir_path[dir_len] = '\0';
        strcpy(file_prefix, last_slash + 1);
    } else {
        strcpy(dir_path, "./");
        strcpy(file_prefix, partial_path);
    }
    
    DIR *dir = opendir(dir_path);
    if (!dir) {
        log_message(LOG_DEBUG, "Cannot open directory: %s", dir_path);
        return NULL;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && *match_count < MAX_MATCHES) {
        // 跳过隐藏文件（除非明确输入点）
        if (file_prefix[0] != '.' && entry->d_name[0] == '.') {
            continue;
        }
        
        // 简单前缀匹配（可替换为汇编优化版本）
        if (strncmp(entry->d_name, file_prefix, strlen(file_prefix)) == 0) {
            char *full_path = malloc(1024);
            if (full_path) {
                snprintf(full_path, 1024, "%s%s", dir_path, entry->d_name);
                
                // 如果是目录，添加斜杠
                if (entry->d_type == DT_DIR) {
                    strcat(full_path, "/");
                }
                
                matches[(*match_count)++] = full_path;
            }
        }
    }
    
    closedir(dir);
    
    // 简单排序
    if (*match_count > 1) {
        qsort(matches, *match_count, sizeof(char*), 
            (int(*)(const void*, const void*))strcmp);
    }
    
    return matches;
}