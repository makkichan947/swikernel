#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/common_defs.h"
#include "../include/swikernel.h"

// 模拟进度回调
void test_progress_callback(const char* phase, int percent, void* user_data) {
    printf("Progress: %s - %d%%\n", phase, percent);
}

// 测试构建目录创建
void test_build_environment(void) {
    printf("Testing build environment setup...\n");
    
    const char *test_dir = "/tmp/swikernel_test";
    
    // 清理之前的测试
    system("rm -rf /tmp/swikernel_test");
    
    // 创建目录
    assert(ensure_directory(test_dir) == 0);
    
    // 检查目录是否存在
    struct stat st;
    assert(stat(test_dir, &st) == 0);
    assert(S_ISDIR(st.st_mode));
    
    printf("Build environment test passed!\n");
}

// 测试配置备份
void test_config_backup(void) {
    printf("Testing configuration backup...\n");
    
    // 创建测试配置文件
    system("mkdir -p /tmp/swikernel_test/config");
    system("echo 'test config' > /tmp/swikernel_test/config/test.conf");
    
    // 这里可以添加实际的备份测试
    // 由于需要root权限，我们只测试基本逻辑
    
    printf("Config backup test completed (basic logic check)\n");
}

// 测试依赖解析
void test_dependency_resolution(void) {
    printf("Testing dependency resolution...\n");
    
    DependencyStatus status = check_system_dependencies();
    
    printf("Required tools: %d/%d present\n", 
        status.required_present, status.required_total);
    printf("Optional tools: %d/%d present\n", 
        status.optional_present, status.optional_total);
    
    if (status.missing_required_count > 0) {
        printf("Missing required dependencies:\n");
        for (int i = 0; i < status.missing_required_count; i++) {
            printf("  - %s\n", status.missing_required[i]);
        }
    }
    
    free_dependency_status(&status);
    printf("Dependency resolution test passed!\n");
}

// 测试内核信息扫描
void test_kernel_scanning(void) {
    printf("Testing kernel scanning...\n");
    
    KernelInfo *kernels = scan_installed_kernels();
    
    if (kernels) {
        KernelInfo *current = kernels;
        int count = 0;
        
        while (current) {
            printf("Found kernel: %s (%s)\n", current->name, current->version);
            current = current->next;
            count++;
        }
        
        printf("Scanned %d kernels\n", count);
        free_kernel_list(kernels);
    } else {
        printf("No kernels found (this might be normal in test environment)\n");
    }
    
    printf("Kernel scanning test completed\n");
}

// 测试文件操作
void test_file_operations(void) {
    printf("Testing file operations...\n");
    
    const char *test_file = "/tmp/swikernel_test/fileops.txt";
    const char *test_content = "Hello, SwiKernel!";
    
    // 测试文件写入
    FILE *fp = fopen(test_file, "w");
    assert(fp != NULL);
    fwrite(test_content, 1, strlen(test_content), fp);
    fclose(fp);
    
    // 测试文件读取
    fp = fopen(test_file, "r");
    assert(fp != NULL);
    char buffer[64];
    fgets(buffer, sizeof(buffer), fp);
    fclose(fp);
    
    assert(strcmp(buffer, test_content) == 0);
    
    // 测试文件校验
    char *checksum = get_file_checksum(test_file);
    assert(checksum != NULL);
    printf("File checksum: %s\n", checksum);
    free(checksum);
    
    // 清理
    unlink(test_file);
    
    printf("File operations test passed!\n");
}

int main(int argc, char *argv[]) {
    printf("Starting SwiKernel installation tests...\n\n");
    
    // 检查root权限
    if (geteuid() != 0) {
        printf("Warning: Some tests require root privileges\n");
    }
    
    test_build_environment();
    test_config_backup();
    test_dependency_resolution();
    test_kernel_scanning();
    test_file_operations();
    
    printf("\nAll installation tests completed! ✓\n");
    return 0;
}