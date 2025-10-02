#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>
#include "system_monitor.h"
#include "logger.h"

// 初始化系统监控器
int system_monitor_init(SystemMonitor *sm) {
    if (!sm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    memset(sm, 0, sizeof(SystemMonitor));

    // 设置默认配置
    system_monitor_set_default_config(sm);

    // 分配历史记录缓冲区
    sm->history_stats = malloc(sizeof(SystemStats) * sm->config.history_size);
    if (!sm->history_stats) {
        return SWK_ERROR_OUT_OF_MEMORY;
    }

    memset(sm->history_stats, 0, sizeof(SystemStats) * sm->config.history_size);

    // 采集初始统计信息
    if (system_monitor_update(sm) != SWK_SUCCESS) {
        log_message(LOG_WARNING, "Failed to collect initial system stats");
    }

    log_message(LOG_DEBUG, "System monitor initialized");
    return SWK_SUCCESS;
}

// 清理系统监控器
void system_monitor_cleanup(SystemMonitor *sm) {
    if (!sm) return;

    if (sm->history_stats) {
        free(sm->history_stats);
        sm->history_stats = NULL;
    }

    sm->running = 0;
    sm->history_count = 0;
    sm->history_index = 0;
}

// 设置默认配置
void system_monitor_set_default_config(SystemMonitor *sm) {
    if (!sm) return;

    sm->config.update_interval = 2;
    sm->config.history_size = 100;
    sm->config.enable_cpu_monitor = 1;
    sm->config.enable_memory_monitor = 1;
    sm->config.enable_disk_monitor = 1;
    sm->config.enable_network_monitor = 1;
    sm->config.enable_process_monitor = 1;
    strcpy(sm->config.disk_device, "/");
    strcpy(sm->config.network_interface, "eth0");
}

// 更新系统统计信息
int system_monitor_update(SystemMonitor *sm) {
    if (!sm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    SystemStats *current = &sm->current_stats;
    memset(current, 0, sizeof(SystemStats));
    current->timestamp = time(NULL);

    int result = SWK_SUCCESS;

    // 采集各种统计信息
    if (sm->config.enable_cpu_monitor) {
        if (system_monitor_collect_cpu_stats(current) != SWK_SUCCESS) {
            result = SWK_ERROR;
        }
    }

    if (sm->config.enable_memory_monitor) {
        if (system_monitor_collect_memory_stats(current) != SWK_SUCCESS) {
            result = SWK_ERROR;
        }
    }

    if (sm->config.enable_disk_monitor) {
        if (system_monitor_collect_disk_stats(current, sm->config.disk_device) != SWK_SUCCESS) {
            result = SWK_ERROR;
        }
    }

    if (sm->config.enable_process_monitor) {
        if (system_monitor_collect_process_stats(current) != SWK_SUCCESS) {
            result = SWK_ERROR;
        }
    }

    if (sm->config.enable_network_monitor) {
        if (system_monitor_collect_network_stats(current, sm->config.network_interface) != SWK_SUCCESS) {
            result = SWK_ERROR;
        }
    }

    // 采集系统负载
    if (system_monitor_collect_load_stats(current) != SWK_SUCCESS) {
        result = SWK_ERROR;
    }

    // 保存到历史记录
    if (sm->history_stats) {
        memcpy(&sm->history_stats[sm->history_index], current, sizeof(SystemStats));

        sm->history_index = (sm->history_index + 1) % sm->config.history_size;
        if (sm->history_count < sm->config.history_size) {
            sm->history_count++;
        }
    }

    sm->last_update = time(NULL);
    return result;
}

// 采集CPU统计信息
int system_monitor_collect_cpu_stats(SystemStats *stats) {
    if (!stats) {
        return SWK_ERROR_INVALID_PARAM;
    }

    FILE *file = fopen("/proc/stat", "r");
    if (!file) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    char line[256];
    unsigned long user, nice, system, idle, iowait, irq, softirq;

    // 读取第一行（总CPU统计）
    if (fgets(line, sizeof(line), file)) {
        sscanf(line, "cpu %lu %lu %lu %lu %lu %lu %lu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq);

        unsigned long total = user + nice + system + idle + iowait + irq + softirq;
        unsigned long active = total - idle;

        if (total > 0) {
            stats->cpu_usage = (double)active * 100.0 / total;
        }
    }

    fclose(file);

    // 获取CPU核心数
    file = fopen("/proc/cpuinfo", "r");
    if (file) {
        char cpuinfo_line[256];
        while (fgets(cpuinfo_line, sizeof(cpuinfo_line), file)) {
            if (strncmp(cpuinfo_line, "cpu cores", 9) == 0) {
                sscanf(cpuinfo_line, "cpu cores : %d", &stats->cpu_cores);
                break;
            }
        }
        fclose(file);
    }

    // 尝试读取CPU频率（如果可用）
    file = fopen("/proc/cpuinfo", "r");
    if (file) {
        char cpuinfo_line[256];
        while (fgets(cpuinfo_line, sizeof(cpuinfo_line), file)) {
            if (strncmp(cpuinfo_line, "cpu MHz", 7) == 0) {
                sscanf(cpuinfo_line, "cpu MHz : %lf", &stats->cpu_frequency);
                break;
            }
        }
        fclose(file);
    }

    return SWK_SUCCESS;
}

// 采集内存统计信息
int system_monitor_collect_memory_stats(SystemStats *stats) {
    if (!stats) {
        return SWK_ERROR_INVALID_PARAM;
    }

    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        return SWK_ERROR_SYSTEM_CALL;
    }

    stats->memory_total = info.totalram * info.mem_unit / 1024;
    stats->memory_free = info.freeram * info.mem_unit / 1024;
    stats->memory_used = stats->memory_total - stats->memory_free;
    stats->memory_buffers = info.bufferram * info.mem_unit / 1024;

    // 内存使用率
    if (stats->memory_total > 0) {
        stats->memory_usage_percent = (double)stats->memory_used * 100.0 / stats->memory_total;
    }

    // 交换分区信息
    stats->swap_total = info.totalswap * info.mem_unit / 1024;
    stats->swap_free = info.freeswap * info.mem_unit / 1024;
    stats->swap_used = stats->swap_total - stats->swap_free;

    if (stats->swap_total > 0) {
        stats->swap_usage_percent = (double)stats->swap_used * 100.0 / stats->swap_total;
    }

    return SWK_SUCCESS;
}

// 采集磁盘统计信息
int system_monitor_collect_disk_stats(SystemStats *stats, const char *device) {
    if (!stats || !device) {
        return SWK_ERROR_INVALID_PARAM;
    }

    struct statvfs fs;
    if (statvfs(device, &fs) != 0) {
        return SWK_ERROR_SYSTEM_CALL;
    }

    unsigned long block_size = fs.f_frsize;
    stats->disk_total = (unsigned long)fs.f_blocks * block_size / 1024;
    stats->disk_free = (unsigned long)fs.f_bavail * block_size / 1024;
    stats->disk_used = stats->disk_total - stats->disk_free;

    if (stats->disk_total > 0) {
        stats->disk_usage_percent = (double)stats->disk_used * 100.0 / stats->disk_total;
    }

    strncpy(stats->disk_mount_point, device, sizeof(stats->disk_mount_point) - 1);

    return SWK_SUCCESS;
}

// 采集进程统计信息
int system_monitor_collect_process_stats(SystemStats *stats) {
    if (!stats) {
        return SWK_ERROR_INVALID_PARAM;
    }

    DIR *dir = opendir("/proc");
    if (!dir) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    struct dirent *entry;
    int process_count = 0;
    int thread_count = 0;
    int zombie_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // 检查是否为进程目录（数字目录名）
        if (isdigit(entry->d_name[0])) {
            process_count++;

            // 读取进程状态
            char stat_path[256];
            snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat", entry->d_name);

            FILE *stat_file = fopen(stat_path, "r");
            if (stat_file) {
                char stat_line[1024];
                if (fgets(stat_line, sizeof(stat_line), stat_file)) {
                    // 解析进程状态
                    char state;
                    int threads;
                    sscanf(stat_line, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %d",
                           &state, &threads);

                    thread_count += threads;

                    if (state == 'Z') {
                        zombie_count++;
                    }
                }
                fclose(stat_file);
            }
        }
    }

    closedir(dir);

    stats->process_count = process_count;
    stats->thread_count = thread_count;
    stats->zombie_processes = zombie_count;

    return SWK_SUCCESS;
}

// 采集网络统计信息
int system_monitor_collect_network_stats(SystemStats *stats, const char *interface) {
    if (!stats || !interface) {
        return SWK_ERROR_INVALID_PARAM;
    }

    char path[256];
    snprintf(path, sizeof(path), "/proc/net/dev");

    FILE *file = fopen(path, "r");
    if (!file) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    char line[256];
    unsigned long rx_bytes = 0, tx_bytes = 0;

    // 跳过标题行
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file)) {
        char iface[32];
        unsigned long rx_packets, rx_errors, rx_drop, rx_fifo, rx_frame;
        unsigned long tx_packets, tx_errors, tx_drop, tx_fifo, tx_colls, tx_carrier;

        // 解析网络接口统计
        if (sscanf(line, "%31s %lu %lu %lu %lu %lu %lu %*u %*u %lu %lu %lu %lu %lu %lu %lu",
                   iface, &rx_bytes, &rx_packets, &rx_errors, &rx_drop, &rx_fifo, &rx_frame,
                   &tx_bytes, &tx_packets, &tx_errors, &tx_drop, &tx_fifo, &tx_colls, &tx_carrier) == 14) {

            // 移除接口名后的冒号
            char *colon = strchr(iface, ':');
            if (colon) *colon = '\0';

            if (strcmp(iface, interface) == 0) {
                stats->network_rx_bytes = rx_bytes;
                stats->network_tx_bytes = tx_bytes;
                break;
            }
        }
    }

    fclose(file);
    return SWK_SUCCESS;
}

// 采集系统负载统计
int system_monitor_collect_load_stats(SystemStats *stats) {
    if (!stats) {
        return SWK_ERROR_INVALID_PARAM;
    }

    FILE *file = fopen("/proc/loadavg", "r");
    if (!file) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    if (fscanf(file, "%lf %lf %lf",
               &stats->load_average_1,
               &stats->load_average_5,
               &stats->load_average_15) != 3) {
        fclose(file);
        return SWK_ERROR;
    }

    fclose(file);
    return SWK_SUCCESS;
}

// 获取历史统计信息
SystemStats *system_monitor_get_history(SystemMonitor *sm, int *count) {
    if (!sm || !count) {
        return NULL;
    }

    *count = sm->history_count;
    return sm->history_stats;
}

// 打印统计信息
void system_monitor_print_stats(SystemStats *stats) {
    if (!stats) return;

    printf("=== 系统统计信息 ===\n");
    printf("时间戳: %s", ctime(&stats->timestamp));
    printf("CPU使用率: %.1f%% (核心数: %d, 频率: %.0f MHz)\n",
           stats->cpu_usage, stats->cpu_cores, stats->cpu_frequency);
    printf("内存使用: %lu/%lu KB (%.1f%%)\n",
           stats->memory_used, stats->memory_total, stats->memory_usage_percent);
    printf("交换分区: %lu/%lu KB (%.1f%%)\n",
           stats->swap_used, stats->swap_total, stats->swap_usage_percent);
    printf("磁盘使用: %lu/%lu KB (%.1f%%) [%s]\n",
           stats->disk_used, stats->disk_total, stats->disk_usage_percent, stats->disk_mount_point);
    printf("网络流量: RX=%lu bytes, TX=%lu bytes\n",
           stats->network_rx_bytes, stats->network_tx_bytes);
    printf("进程统计: %d 进程, %d 线程, %d 僵尸进程\n",
           stats->process_count, stats->thread_count, stats->zombie_processes);
    printf("系统负载: %.2f, %.2f, %.2f\n",
           stats->load_average_1, stats->load_average_5, stats->load_average_15);
}

// 打印紧凑统计信息
void system_monitor_print_compact_stats(SystemStats *stats) {
    if (!stats) return;

    printf("CPU:%.1f%% MEM:%.1f%% DISK:%.1f%% LOAD:%.2f ",
           stats->cpu_usage,
           stats->memory_usage_percent,
           stats->disk_usage_percent,
           stats->load_average_1);
}

// 获取指定时间的统计信息
SystemStats *system_monitor_get_stats_at_time(SystemMonitor *sm, time_t timestamp) {
    if (!sm || !sm->history_stats) {
        return NULL;
    }

    // 在历史记录中查找最接近的时间戳
    SystemStats *closest = NULL;
    time_t min_diff = (time_t)-1;

    for (int i = 0; i < sm->history_count; i++) {
        time_t diff = abs(sm->history_stats[i].timestamp - timestamp);
        if (diff < min_diff) {
            min_diff = diff;
            closest = &sm->history_stats[i];
        }
    }

    return closest;
}

// 获取平均值
double system_monitor_get_average_value(SystemMonitor *sm, int offset, size_t field_offset) {
    if (!sm || !sm->history_stats || offset >= sm->history_count) {
        return 0.0;
    }

    double sum = 0.0;
    int count = 0;

    for (int i = 0; i < sm->history_count && i < offset; i++) {
        double *value = (double*)((char*)&sm->history_stats[i] + field_offset);
        sum += *value;
        count++;
    }

    return count > 0 ? sum / count : 0.0;
}