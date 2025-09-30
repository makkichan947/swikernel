// src/system/process.c
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include "system.h"
#include "logger.h"

// 执行命令并返回退出状态
int execute_command(const char *command) {
    log_message(LOG_DEBUG, "Executing command: %s", command);
    
    int status = system(command);
    if (status == -1) {
        log_message(LOG_ERROR, "Failed to execute command: %s", command);
        return -1;
    }
    
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (exit_status != 0) {
            log_message(LOG_WARNING, "Command exited with status %d: %s", exit_status, command);
        }
        return exit_status;
    } else if (WIFSIGNALED(status)) {
        log_message(LOG_ERROR, "Command terminated by signal %d: %s", WTERMSIG(status), command);
        return -1;
    } else {
        log_message(LOG_ERROR, "Command terminated abnormally: %s", command);
        return -1;
    }
}

// 执行命令并捕获输出
char* execute_command_capture(const char *command) {
    FILE *fp;
    char *output = NULL;
    size_t output_size = 0;
    size_t total_size = 0;
    char buffer[1024];

    log_message(LOG_DEBUG, "Executing command with output capture: %s", command);
    
    fp = popen(command, "r");
    if (fp == NULL) {
        log_message(LOG_ERROR, "Failed to popen command: %s", command);
        return NULL;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t buffer_len = strlen(buffer);
        char *new_output = realloc(output, total_size + buffer_len + 1);
        if (new_output == NULL) {
            log_message(LOG_ERROR, "Failed to allocate memory for command output");
            free(output);
            pclose(fp);
            return NULL;
        }
        output = new_output;
        memcpy(output + total_size, buffer, buffer_len);
        total_size += buffer_len;
        output[total_size] = '\0';
    }

    int status = pclose(fp);
    if (status != 0) {
        log_message(LOG_WARNING, "Command exited with status %d: %s", status, command);
        // 不立即返回，可能仍然需要输出
    }

    return output;
}

// 后台执行命令
pid_t execute_background(const char *command) {
    pid_t pid = fork();
    if (pid == 0) {
        // 子进程
        setsid();
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
        execl("/bin/sh", "sh", "-c", command, NULL);
        exit(1); // 如果execl失败
    } else if (pid > 0) {
        log_message(LOG_DEBUG, "Started background process %d: %s", pid, command);
        return pid;
    } else {
        log_message(LOG_ERROR, "Failed to fork for background command: %s", command);
        return -1;
    }
}

// 等待进程完成
int wait_for_process(pid_t pid, int timeout_seconds) {
    int status;
    time_t start_time = time(NULL);
    
    while (1) {
        pid_t result = waitpid(pid, &status, WNOHANG);
        if (result == pid) {
            // 进程已结束
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            } else {
                return -1;
            }
        } else if (result == 0) {
            // 进程仍在运行
            if (timeout_seconds > 0 && (time(NULL) - start_time) > timeout_seconds) {
                log_message(LOG_WARNING, "Process %d timeout, sending SIGTERM", pid);
                kill(pid, SIGTERM);
                sleep(2);
                if (waitpid(pid, &status, WNOHANG) == 0) {
                    kill(pid, SIGKILL);
                }
                return -2; // 超时
            }
            sleep(1);
        } else {
            // 错误
            log_message(LOG_ERROR, "Error waiting for process %d", pid);
            return -1;
        }
    }
}