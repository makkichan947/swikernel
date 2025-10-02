#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include <stddef.h>
#include <stdint.h>

// 平台检测
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_LINUX 0
#elif defined(__linux__)
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX 1
#else
    #define PLATFORM_WINDOWS 0
    #define PLATFORM_LINUX 0
#endif

// 编译器特性
#if defined(__GNUC__)
    #define FORCE_INLINE __attribute__((always_inline)) inline
    #define PACKED __attribute__((packed))
#elif defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
    #define PACKED
#else
    #define FORCE_INLINE inline
    #define PACKED
#endif

// 通用常量
#define MAX_PATH_LENGTH 1024
#define MAX_KERNEL_NAME_LENGTH 128
#define MAX_VERSION_LENGTH 64
#define MAX_BUFFER_SIZE 4096
#define MAX_LOG_SIZE (10 * 1024 * 1024) // 10MB

// 返回代码
typedef enum {
    SWK_SUCCESS = 0,
    SWK_ERROR = -1,
    SWK_ERROR_INVALID_PARAM = -2,
    SWK_ERROR_FILE_NOT_FOUND = -3,
    SWK_ERROR_PERMISSION_DENIED = -4,
    SWK_ERROR_OUT_OF_MEMORY = -5,
    SWK_ERROR_SYSTEM_CALL = -6,
    SWK_ERROR_COMPILATION = -7,
    SWK_ERROR_DEPENDENCY = -8,
    SWK_ERROR_ROLLBACK = -9
} SwkReturnCode;

// 布尔类型
typedef enum {
    SWK_FALSE = 0,
    SWK_TRUE = 1
} SwkBool;

// 内核架构
typedef enum {
    ARCH_X86_64,
    ARCH_ARM64,
    ARCH_ARM,
    ARCH_PPC64LE,
    ARCH_RISCV64,
    ARCH_UNKNOWN
} KernelArchitecture;

// 内核类型
typedef enum {
    KERNEL_TYPE_STABLE,
    KERNEL_TYPE_LTS,
    KERNEL_TYPE_MAINLINE,
    KERNEL_TYPE_RT,
    KERNEL_TYPE_CUSTOM
} KernelType;

// 日志级别已在logger.h中定义，避免重复定义

// 进度回调函数类型
typedef void (*ProgressCallback)(const char* phase, int percent, void* user_data);

// 通用结果结构
typedef struct {
    SwkReturnCode code;
    char message[256];
    void* data;
} SwkResult;

// 内存统计
typedef struct {
    size_t total;
    size_t used;
    size_t free;
    size_t cached;
} MemoryStats;

// CPU 统计
typedef struct {
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
    unsigned long iowait;
    unsigned long irq;
    unsigned long softirq;
} CpuStats;

// 系统信息
typedef struct {
    char hostname[256];
    char os_name[256];
    char os_version[256];
    char architecture[64];
    int cpu_cores;
    MemoryStats memory;
    uint64_t uptime;
} SystemInfo;

// 通用函数
FORCE_INLINE size_t min_size(size_t a, size_t b) {
    return (a < b) ? a : b;
}

FORCE_INLINE size_t max_size(size_t a, size_t b) {
    return (a > b) ? a : b;
}

// 字符串安全复制
#define STRNCPY(dest, src, size) do { \
    strncpy((dest), (src), (size) - 1); \
    (dest)[(size) - 1] = '\0'; \
} while (0)

// 断言宏
#ifdef DEBUG
    #define SWK_ASSERT(condition) do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", \
                    #condition, __FILE__, __LINE__); \
            abort(); \
        } \
    } while (0)
#else
    #define SWK_ASSERT(condition) ((void)0)
#endif

// 调试输出宏
#ifdef DEBUG
    #define DBG_PRINT(fmt, ...) \
        fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define DBG_PRINT(fmt, ...) ((void)0)
#endif

#endif // COMMON_DEFS_H