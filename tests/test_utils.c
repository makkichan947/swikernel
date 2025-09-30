#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/common_defs.h"
#include "../src/utils/logger.h"
#include "../src/utils/error_handler.h"

// 测试日志系统
void test_logging_system(void) {
    printf("Testing logging system...\n");
    
    // 初始化日志
    assert(logger_init("/tmp/swikernel_test.log", LOG_LEVEL_DEBUG, 0) == 0);
    
    // 测试各种日志级别
    log_message(LOG_LEVEL_DEBUG, "This is a debug message");
    log_message(LOG_LEVEL_INFO, "This is an info message");
    log_message(LOG_LEVEL_WARNING, "This is a warning message");
    log_message(LOG_LEVEL_ERROR, "This is an error message");
    
    // 测试日志级别设置
    logger_set_level(LOG_LEVEL_WARNING);
    assert(logger_get_level() == LOG_LEVEL_WARNING);
    
    // 清理
    logger_cleanup();
    
    printf("Logging system test passed!\n");
}

// 测试错误处理
void test_error_handling(void) {
    printf("Testing error handling...\n");
    
    // 测试错误报告
    report_error(ERROR_WARNING, "Test warning", __FILE__, __LINE__);
    report_error(ERROR_ERROR, "Test error", __FILE__, __LINE__);
    
    // 测试回滚系统
    FileBackupData backup_data;
    strcpy(backup_data.backup_path, "/tmp/backup");
    strcpy(backup_data.original_path, "/tmp/original");
    
    assert(add_rollback_step(ACTION_RESTORE_FILE, &backup_data, sizeof(backup_data)) == 0);
    
    // 执行回滚（应该正常完成）
    execute_rollback();
    
    printf("Error handling test passed!\n");
}

// 测试内存操作
void test_memory_operations(void) {
    printf("Testing memory operations...\n");
    
    // 测试内存分配
    void *ptr = malloc(1024);
    assert(ptr != NULL);
    
    // 测试内存设置
    memset(ptr, 0xAA, 1024);
    
    // 检查设置是否正确
    unsigned char *byte_ptr = (unsigned char*)ptr;
    for (int i = 0; i < 1024; i++) {
        assert(byte_ptr[i] == 0xAA);
    }
    
    free(ptr);
    
    printf("Memory operations test passed!\n");
}

// 测试字符串操作
void test_string_operations(void) {
    printf("Testing string operations...\n");
    
    char buffer[64];
    
    // 测试安全复制
    STRNCPY(buffer, "Hello, World!", sizeof(buffer));
    assert(strcmp(buffer, "Hello, World!") == 0);
    
    // 测试长字符串截断
    STRNCPY(buffer, "This is a very long string that should be truncated", sizeof(buffer));
    assert(strlen(buffer) < sizeof(buffer));
    assert(buffer[sizeof(buffer)-1] == '\0');
    
    printf("String operations test passed!\n");
}

// 测试路径操作
void test_path_operations(void) {
    printf("Testing path operations...\n");
    
    // 测试路径扩展
    char *home_path = expand_path("~");
    assert(home_path != NULL);
    assert(strlen(home_path) > 0);
    free(home_path);
    
    // 测试绝对路径
    char *abs_path = expand_path("/absolute/path");
    assert(strcmp(abs_path, "/absolute/path") == 0);
    free(abs_path);
    
    // 测试目录创建
    assert(ensure_directory("/tmp/swikernel_test_dirs/subdir") == 0);
    
    struct stat st;
    assert(stat("/tmp/swikernel_test_dirs/subdir", &st) == 0);
    assert(S_ISDIR(st.st_mode));
    
    // 清理
    system("rm -rf /tmp/swikernel_test_dirs");
    
    printf("Path operations test passed!\n");
}

// 测试校验和计算
void test_checksum_calculation(void) {
    printf("Testing checksum calculation...\n");
    
    const char *test_file = "/tmp/swikernel_test_checksum.txt";
    const char *test_data = "Test data for checksum calculation";
    
    // 创建测试文件
    FILE *fp = fopen(test_file, "w");
    assert(fp != NULL);
    fwrite(test_data, 1, strlen(test_data), fp);
    fclose(fp);
    
    // 计算校验和
    char *checksum = get_file_checksum(test_file);
    assert(checksum != NULL);
    assert(strlen(checksum) > 0);
    
    printf("Checksum for test file: %s\n", checksum);
    
    free(checksum);
    unlink(test_file);
    
    printf("Checksum calculation test passed!\n");
}

int main(int argc, char *argv[]) {
    printf("Starting SwiKernel utility tests...\n\n");
    
    test_logging_system();
    test_error_handling();
    test_memory_operations();
    test_string_operations();
    test_path_operations();
    test_checksum_calculation();
    
    printf("\nAll utility tests passed! ✓\n");
    return 0;
}