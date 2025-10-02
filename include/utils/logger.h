#ifndef LOGGER_H
#define LOGGER_H

#include "../common_defs.h"

// 日志级别
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

// 日志初始化
int logger_init(const char *filename, LogLevel level, int rotate);
void log_message(LogLevel level, const char *format, ...);
void logger_cleanup(void);
void logger_set_level(LogLevel level);
LogLevel logger_get_level(void);
void rotate_log_files(void);

#endif