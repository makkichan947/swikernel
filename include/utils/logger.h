#ifndef LOGGER_H
#define LOGGER_H

#include "../common_defs.h"

// 日志级别
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO, 
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} LogLevel;

// 日志初始化
int logger_init(const char *filename, LogLevel level, int rotate);
void log_message(LogLevel level, const char *format, ...);
void logger_cleanup(void);
void logger_set_level(LogLevel level);
LogLevel logger_get_level(void);
void rotate_log_files(void);

#endif