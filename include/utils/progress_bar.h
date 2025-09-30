#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include "../common_defs.h"

// 进度条类型
typedef enum {
    PROGRESS_SIMPLE = 0,
    PROGRESS_BLOCK = 1
} ProgressBarType;

// 进度显示函数
void show_progress_bar(const char *label, int current, int total, ProgressBarType type);
void show_spinner(const char *label, int *counter);
void show_download_progress(const char *label, long long current, long long total, 
                        double speed, time_t start_time);
void show_multi_progress(const char **labels, const int *currents, const int *totals, int count);

#endif