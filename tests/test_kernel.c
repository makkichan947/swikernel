#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/common_defs.h"
#include "../include/swikernel.h"

// 测试内核链表功能
void test_kernel_list(void) {
    printf("Testing kernel list functionality...\n");
    
    KernelInfo *list = NULL;
    KernelInfo *k1 = malloc(sizeof(KernelInfo));
    KernelInfo *k2 = malloc(sizeof(KernelInfo));
    
    // 初始化测试数据
    memset(k1, 0, sizeof(KernelInfo));
    memset(k2, 0, sizeof(KernelInfo));
    
    strcpy(k1->name, "test-kernel-5.15");
    strcpy(k1->version, "5.15.0");
    strcpy(k1->arch, "x86_64");
    k1->type = KERNEL_TYPE_STABLE;
    k1->installed = 1;
    k1->is_running = 0;
    
    strcpy(k2->name, "test-kernel-6.1");
    strcpy(k2->version, "6.1.0");
    strcpy(k2->arch, "x86_64");
    k2->type = KERNEL_TYPE_LTS;
    k2->installed = 1;
    k2->is_running = 1;
    
    // 构建链表
    k1->next = k2;
    list = k1;
    
    // 测试查找功能
    KernelInfo *found = find_kernel_by_name("test-kernel-5.15");
    assert(found != NULL);
    assert(strcmp(found->name, "test-kernel-5.15") == 0);
    
    found = find_kernel_by_name("nonexistent");
    assert(found == NULL);
    
    // 测试释放
    free_kernel_list(list);
    
    printf("Kernel list tests passed!\n");
}

// 测试配置加载
void test_config_loading(void) {
    printf("Testing configuration loading...\n");
    
    SwikernelConfig config;
    set_default_config(&config);
    
    assert(strlen(config.log_file) > 0);
    assert(config.log_level >= LOG_LEVEL_DEBUG && config.log_level <= LOG_LEVEL_FATAL);
    assert(config.backup_enabled == 0 || config.backup_enabled == 1);
    assert(config.auto_dependencies == 0 || config.auto_dependencies == 1);
    
    printf("Config loading tests passed!\n");
}

// 测试路径处理
void test_path_handling(void) {
    printf("Testing path handling...\n");
    
    char *expanded = expand_path("~/.config/swikernel");
    assert(expanded != NULL);
    assert(strstr(expanded, "/") != NULL);
    free(expanded);
    
    expanded = expand_path("/absolute/path");
    assert(strcmp(expanded, "/absolute/path") == 0);
    free(expanded);
    
    printf("Path handling tests passed!\n");
}

// 测试依赖检查
void test_dependency_checking(void) {
    printf("Testing dependency checking...\n");
    
    DependencyStatus status = check_system_dependencies();
    
    // 基本检查
    assert(status.required_total >= 0);
    assert(status.required_present >= 0);
    assert(status.required_present <= status.required_total);
    
    printf("Dependency checking tests passed!\n");
    free_dependency_status(&status);
}

// 测试工具函数
void test_utility_functions(void) {
    printf("Testing utility functions...\n");
    
    // 测试最小/最大函数
    assert(min_size(10, 20) == 10);
    assert(min_size(20, 10) == 10);
    assert(max_size(10, 20) == 20);
    assert(max_size(20, 10) == 20);
    
    // 测试字符串安全复制
    char dest[10];
    STRNCPY(dest, "hello world", sizeof(dest));
    assert(strlen(dest) < sizeof(dest));
    assert(dest[sizeof(dest)-1] == '\0');
    
    printf("Utility function tests passed!\n");
}

// 主测试函数
int main(int argc, char *argv[]) {
    printf("Starting SwiKernel test suite...\n\n");
    
    int run_specific_test = 0;
    char *test_name = NULL;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test") == 0 && i + 1 < argc) {
            run_specific_test = 1;
            test_name = argv[++i];
        }
    }
    
    int all_passed = 1;
    
    // 运行测试
    if (!run_specific_test || (test_name && strcmp(test_name, "kernel_list") == 0)) {
        test_kernel_list();
    }
    
    if (!run_specific_test || (test_name && strcmp(test_name, "config") == 0)) {
        test_config_loading();
    }
    
    if (!run_specific_test || (test_name && strcmp(test_name, "path") == 0)) {
        test_path_handling();
    }
    
    if (!run_specific_test || (test_name && strcmp(test_name, "deps") == 0)) {
        test_dependency_checking();
    }
    
    if (!run_specific_test || (test_name && strcmp(test_name, "utils") == 0)) {
        test_utility_functions();
    }
    
    if (all_passed) {
        printf("\nAll tests passed! ✓\n");
        return 0;
    } else {
        printf("\nSome tests failed! ✗\n");
        return 1;
    }
}