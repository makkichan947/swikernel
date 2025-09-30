#ifndef DEPENDENCY_CHECK_H
#define DEPENDENCY_CHECK_H

#include "../common_defs.h"

// 依赖状态结构
typedef struct {
    int required_total;
    int required_present;
    int optional_total;
    int optional_present;
    int missing_required_count;
    int missing_optional_count;
    char **missing_required;
    char **missing_optional;
} DependencyStatus;

// 依赖检查函数
DependencyStatus check_system_dependencies(void);
int check_tool_exists(const char *tool);
char* get_tool_version(const char *tool);
void show_dependency_status(DependencyStatus *status);
int install_missing_dependencies(DependencyStatus *status);
void free_dependency_status(DependencyStatus *status);

#endif