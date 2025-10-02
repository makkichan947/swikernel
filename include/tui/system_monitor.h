#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include "../common_defs.h"

// 系统统计信息结构
typedef struct {
    // CPU 信息
    double cpu_usage;           // CPU 使用率 (%)
    int cpu_cores;             // CPU 核心数
    double cpu_frequency;      // CPU 频率 (MHz)
    double cpu_temperature;    // CPU 温度 (°C)

    // 内存信息
    unsigned long memory_total;    // 总内存 (KB)
    unsigned long memory_used;     // 已用内存 (KB)
    unsigned long memory_free;     // 空闲内存 (KB)
    unsigned long memory_buffers;  // 缓冲区 (KB)
    unsigned long memory_cached;   // 缓存 (KB)
    double memory_usage_percent;   // 内存使用率 (%)

    // 交换分区信息
    unsigned long swap_total;      // 总交换空间 (KB)
    unsigned long swap_used;       // 已用交换空间 (KB)
    unsigned long swap_free;       // 空闲交换空间 (KB)
    double swap_usage_percent;     // 交换空间使用率 (%)

    // 磁盘信息
    unsigned long disk_total;      // 总磁盘空间 (KB)
    unsigned long disk_used;       // 已用磁盘空间 (KB)
    unsigned long disk_free;       // 空闲磁盘空间 (KB)
    double disk_usage_percent;     // 磁盘使用率 (%)
    char disk_mount_point[256];    // 挂载点

    // 网络信息
    unsigned long network_rx_bytes;    // 接收字节数
    unsigned long network_tx_bytes;    // 发送字节数
    double network_rx_rate;           // 接收速率 (KB/s)
    double network_tx_rate;           // 发送速率 (KB/s)

    // 进程信息
    int process_count;         // 进程总数
    int thread_count;          // 线程总数
    int zombie_processes;      // 僵尸进程数

    // 系统负载
    double load_average_1;     // 1分钟平均负载
    double load_average_5;     // 5分钟平均负载
    double load_average_15;    // 15分钟平均负载

    // 时间戳
    time_t timestamp;          // 采集时间
} SystemStats;

// 监控配置
typedef struct {
    int update_interval;       // 更新间隔 (秒)
    int history_size;          // 历史记录大小
    int enable_cpu_monitor;    // 启用CPU监控
    int enable_memory_monitor; // 启用内存监控
    int enable_disk_monitor;   // 启用磁盘监控
    int enable_network_monitor; // 启用网络监控
    int enable_process_monitor; // 启用进程监控
    char disk_device[64];      // 磁盘设备路径
    char network_interface[32]; // 网络接口名称
} MonitorConfig;

// 系统监控器状态
typedef struct {
    SystemStats current_stats;     // 当前统计信息
    SystemStats *history_stats;    // 历史统计信息
    int history_count;             // 历史记录数量
    int history_index;             // 当前历史记录索引
    MonitorConfig config;          // 监控配置
    time_t last_update;            // 最后更新时间
    int running;                   // 运行状态
} SystemMonitor;

// 系统监控器函数
int system_monitor_init(SystemMonitor *sm);
void system_monitor_cleanup(SystemMonitor *sm);
int system_monitor_start(SystemMonitor *sm);
int system_monitor_stop(SystemMonitor *sm);
int system_monitor_update(SystemMonitor *sm);

// 统计信息采集函数
int system_monitor_collect_cpu_stats(SystemStats *stats);
int system_monitor_collect_memory_stats(SystemStats *stats);
int system_monitor_collect_disk_stats(SystemStats *stats, const char *device);
int system_monitor_collect_network_stats(SystemStats *stats, const char *interface);
int system_monitor_collect_process_stats(SystemStats *stats);
int system_monitor_collect_load_stats(SystemStats *stats);

// 配置函数
int system_monitor_load_config(SystemMonitor *sm, const char *config_file);
int system_monitor_save_config(SystemMonitor *sm, const char *config_file);
void system_monitor_set_default_config(SystemMonitor *sm);

// 历史数据管理
SystemStats *system_monitor_get_history(SystemMonitor *sm, int *count);
SystemStats *system_monitor_get_stats_at_time(SystemMonitor *sm, time_t timestamp);
double system_monitor_get_average_value(SystemMonitor *sm, int offset, size_t field_offset);

// 显示函数
void system_monitor_print_stats(SystemStats *stats);
void system_monitor_print_compact_stats(SystemStats *stats);

// TUI 仪表盘函数
void show_system_monitor_dashboard(void);
void show_cpu_monitor_panel(SystemMonitor *sm);
void show_memory_monitor_panel(SystemMonitor *sm);
void show_disk_monitor_panel(SystemMonitor *sm);
void show_network_monitor_panel(SystemMonitor *sm);
void show_process_monitor_panel(SystemMonitor *sm);

// 图表和图形
void system_monitor_draw_cpu_graph(SystemMonitor *sm, int width, int height);
void system_monitor_draw_memory_graph(SystemMonitor *sm, int width, int height);
void system_monitor_draw_load_graph(SystemMonitor *sm, int width, int height);

// 警报和阈值
typedef struct {
    double cpu_warning_threshold;
    double cpu_critical_threshold;
    double memory_warning_threshold;
    double memory_critical_threshold;
    double disk_warning_threshold;
    double disk_critical_threshold;
    int enable_alerts;
    char alert_command[256];
} MonitorAlerts;

int system_monitor_check_alerts(SystemMonitor *sm, MonitorAlerts *alerts);
void system_monitor_set_alerts(SystemMonitor *sm, MonitorAlerts *alerts);

// 导出功能
int system_monitor_export_stats(SystemMonitor *sm, const char *format, const char *output_file);
int system_monitor_export_csv(SystemMonitor *sm, const char *filename);
int system_monitor_export_json(SystemMonitor *sm, const char *filename);

#endif