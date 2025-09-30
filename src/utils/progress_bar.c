// src/utils/progress_bar.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include "progress_bar.h"
#include "logger.h"

// 显示进度条
void show_progress_bar(const char *label, int current, int total, ProgressBarType type) {
    struct winsize w;
    int bar_width = 50;
    
    // 获取终端宽度
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        bar_width = w.ws_col - 40;
        if (bar_width < 20) bar_width = 20;
        if (bar_width > 100) bar_width = 100;
    }
    
    float percentage = (float)current / total;
    int bars = percentage * bar_width;
    
    const char *fill_char = "=";
    const char *empty_char = " ";
    const char *cap_char = ">";
    
    if (type == PROGRESS_BLOCK) {
        fill_char = "█";
        empty_char = "░";
        cap_char = "█";
    }
    
    printf("\r%s: [", label);
    
    for (int i = 0; i < bar_width; i++) {
        if (i < bars) {
            printf("%s", fill_char);
        } else if (i == bars) {
            printf("%s", cap_char);
        } else {
            printf("%s", empty_char);
        }
    }
    
    printf("] %d%% (%d/%d)", (int)(percentage * 100), current, total);
    fflush(stdout);
    
    // 完成后换行
    if (current >= total) {
        printf("\n");
        log_message(LOG_DEBUG, "Progress completed: %s", label);
    }
}

// 显示旋转动画进度
void show_spinner(const char *label, int *counter) {
    const char spinner[] = "|/-\\";
    printf("\r%s: %c ", label, spinner[(*counter)++ % 4]);
    fflush(stdout);
}

// 显示下载进度（带速度和ETA）
void show_download_progress(const char *label, long long current, long long total, 
                        double speed, time_t start_time) {
    struct winsize w;
    int bar_width = 50;
    
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        bar_width = w.ws_col - 60;
        if (bar_width < 20) bar_width = 20;
        if (bar_width > 100) bar_width = 100;
    }
    
    float percentage = (float)current / total;
    int bars = percentage * bar_width;
    
    // 计算ETA
    time_t now = time(NULL);
    time_t elapsed = now - start_time;
    time_t eta = (elapsed / percentage) - elapsed;
    
    // 格式化速度
    const char *unit = "B";
    double display_speed = speed;
    
    if (speed > 1024 * 1024) {
        display_speed = speed / (1024 * 1024);
        unit = "MB";
    } else if (speed > 1024) {
        display_speed = speed / 1024;
        unit = "KB";
    }
    
    printf("\r%s: [", label);
    
    for (int i = 0; i < bar_width; i++) {
        if (i < bars) {
            printf("=");
        } else if (i == bars) {
            printf(">");
        } else {
            printf(" ");
        }
    }
    
    printf("] %d%% %.1f%s/s ETA: %02ld:%02ld", 
           (int)(percentage * 100), display_speed, unit, eta / 60, eta % 60);
    fflush(stdout);
    
    if (current >= total) {
        printf("\n");
        log_message(LOG_DEBUG, "Download completed: %s", label);
    }
}

// 显示多任务进度
void show_multi_progress(const char **labels, const int *currents, const int *totals, int count) {
    printf("\033[%dA", count); // 移动光标到开始
    
    for (int i = 0; i < count; i++) {
        show_progress_bar(labels[i], currents[i], totals[i], PROGRESS_SIMPLE);
        printf("\n");
    }
}