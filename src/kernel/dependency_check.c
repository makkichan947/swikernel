#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dependency_check.h"
#include "logger.h"

// 必需的工具列表
static const char *required_tools[] = {
    "gcc", "make", "bc", "flex", "bison", 
    "libelf-dev", "rsync", "cpio", "xz-utils",
    NULL
};

// 检查工具是否存在
int check_tool_exists(const char *tool) {
    char command[256];
    snprintf(command, sizeof(command), "which %s > /dev/null 2>&1", tool);
    int result = system(command);
    log_message(LOG_DEBUG, "Checking tool %s: %s", tool, result == 0 ? "found" : "missing");
    return result == 0;
}

// 获取工具版本
char* get_tool_version(const char *tool) {
    static char version[256];
    char command[512];
    FILE *pipe;
    
    snprintf(command, sizeof(command), "%s --version 2>/dev/null | head -1", tool);
    
    pipe = popen(command, "r");
    if (!pipe) {
        return NULL;
    }
    
    if (fgets(version, sizeof(version), pipe)) {
        // 移除换行符
        version[strcspn(version, "\n")] = '\0';
        pclose(pipe);
        return version;
    }
    
    pclose(pipe);
    return NULL;
}

// 检查系统依赖
DependencyStatus check_system_dependencies(void) {
    DependencyStatus status = {0};
    
    log_message(LOG_INFO, "Checking system dependencies");
    
    // 检查必需工具
    int missing_count = 0;
    for (int i = 0; required_tools[i]; i++) {
        const char *tool = required_tools[i];
        
        if (check_tool_exists(tool)) {
            char *version = get_tool_version(tool);
            log_message(LOG_DEBUG, "Found required tool: %s (%s)", 
                    tool, version ? version : "unknown");
            status.required_present++;
        } else {
            log_message(LOG_ERROR, "Missing required tool: %s", tool);
            missing_count++;
        }
        status.required_total++;
    }
    
    status.missing_required_count = missing_count;
    
    if (missing_count > 0) {
        log_message(LOG_ERROR, "Missing %d required dependencies", missing_count);
    } else {
        log_message(LOG_INFO, "All required dependencies are satisfied");
    }
    
    return status;
}

// 释放依赖状态内存
void free_dependency_status(DependencyStatus *status) {
    // 当前实现没有动态分配内存
    memset(status, 0, sizeof(DependencyStatus));
}