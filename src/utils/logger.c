// src/utils/logger.c
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include "logger.h"
#include "system.h"

#define LOG_BUFFER_SIZE 4096
#define MAX_LOG_FILES 10
#define MAX_LOG_SIZE (10 * 1024 * 1024) // 10MB

typedef struct {
    FILE *file;
    char filename[256];
    LogLevel level;
    int rotate;
    pthread_mutex_t mutex;
} Logger;

static Logger logger = {0};

// 初始化日志系统
int logger_init(const char *filename, LogLevel level, int rotate) {
    if (logger.file) {
        fclose(logger.file);
    }
    
    logger.file = fopen(filename, "a");
    if (!logger.file) {
        return -1;
    }
    
    strncpy(logger.filename, filename, sizeof(logger.filename) - 1);
    logger.level = level;
    logger.rotate = rotate;
    
    // 初始化互斥锁
    if (pthread_mutex_init(&logger.mutex, NULL) != 0) {
        fclose(logger.file);
        logger.file = NULL;
        return -1;
    }
    
    log_message(LOG_INFO, "Logger initialized (level: %d, file: %s)", level, filename);
    return 0;
}

// 记录日志消息
void log_message(LogLevel level, const char *format, ...) {
    if (level < logger.level || !logger.file) {
        return;
    }
    
    pthread_mutex_lock(&logger.mutex);
    
    // 检查日志文件大小，需要时轮转
    if (logger.rotate) {
        struct stat st;
        if (stat(logger.filename, &st) == 0 && st.st_size > MAX_LOG_SIZE) {
            rotate_log_files();
        }
    }
    
    // 获取高精度时间戳
    struct timeval tv;
    struct tm *tm_info;
    char timestamp[64];
    
    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // 日志级别字符串
    const char *level_str = "DEBUG";
    const char *color_code = "";
    const char *reset_code = "";
    
    switch (level) {
        case LOG_DEBUG: level_str = "DEBUG"; color_code = "\033[36m"; break;
        case LOG_INFO: level_str = "INFO"; color_code = "\033[32m"; break;
        case LOG_WARNING: level_str = "WARN"; color_code = "\033[33m"; break;
        case LOG_ERROR: level_str = "ERROR"; color_code = "\033[31m"; break;
        case LOG_FATAL: level_str = "FATAL"; color_code = "\033[35m"; break;
    }
    
    // 格式化日志消息
    char message[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // 写入日志文件（无颜色）
    fprintf(logger.file, "[%s.%.3ld] [%s] %s\n", 
            timestamp, tv.tv_usec / 1000, level_str, message);
    fflush(logger.file);
    
    // 控制台输出（带颜色）
    fprintf(stderr, "%s[%s.%.3ld] [%s] %s%s\n", 
            color_code, timestamp, tv.tv_usec / 1000, level_str, message, reset_code);
    
    pthread_mutex_unlock(&logger.mutex);
}

// 日志文件轮转
void rotate_log_files(void) {
    fclose(logger.file);
    
    // 轮转现有的日志文件
    for (int i = MAX_LOG_FILES - 1; i > 0; i--) {
        char old_name[256], new_name[256];
        if (i == 1) {
            snprintf(old_name, sizeof(old_name), "%s", logger.filename);
        } else {
            snprintf(old_name, sizeof(old_name), "%s.%d", logger.filename, i - 1);
        }
        snprintf(new_name, sizeof(new_name), "%s.%d", logger.filename, i);
        
        rename(old_name, new_name);
    }
    
    // 重新打开主日志文件
    logger.file = fopen(logger.filename, "w");
    if (logger.file) {
        log_message(LOG_INFO, "Log file rotated successfully");
    }
}

// 清理日志系统
void logger_cleanup(void) {
    pthread_mutex_lock(&logger.mutex);
    
    if (logger.file) {
        log_message(LOG_INFO, "Logger shutting down");
        fclose(logger.file);
        logger.file = NULL;
    }
    
    pthread_mutex_unlock(&logger.mutex);
    pthread_mutex_destroy(&logger.mutex);
}

// 设置日志级别
void logger_set_level(LogLevel level) {
    pthread_mutex_lock(&logger.mutex);
    logger.level = level;
    pthread_mutex_unlock(&logger.mutex);
    
    log_message(LOG_DEBUG, "Log level changed to %d", level);
}

// 获取当前日志级别
LogLevel logger_get_level(void) {
    return logger.level;
}