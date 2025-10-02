#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "../common_defs.h"

// 文件信息结构
typedef struct FileInfo {
    char name[MAX_PATH_LENGTH];
    char path[MAX_PATH_LENGTH];
    char size[32];
    char permissions[16];
    char owner[64];
    char group[64];
    char modified_time[32];
    int is_directory;
    int is_selected;
    struct FileInfo *next;
} FileInfo;

// 文件操作结果
typedef struct {
    SwkReturnCode code;
    char message[256];
    char source_path[MAX_PATH_LENGTH];
    char target_path[MAX_PATH_LENGTH];
} FileOperationResult;

// 文件管理器状态
typedef struct {
    char current_path[MAX_PATH_LENGTH];
    FileInfo *files;
    int file_count;
    int current_selection;
    int scroll_offset;
    int show_hidden;
    char filter_pattern[256];
} FileManager;

// 文件管理器函数
int file_manager_init(FileManager *fm, const char *start_path);
void file_manager_cleanup(FileManager *fm);
int file_manager_refresh(FileManager *fm);
int file_manager_change_directory(FileManager *fm, const char *path);
int file_manager_navigate_up(FileManager *fm);

// 文件操作函数
FileOperationResult file_manager_copy(FileManager *fm, const char *source, const char *destination);
FileOperationResult file_manager_move(FileManager *fm, const char *source, const char *destination);
FileOperationResult file_manager_delete(FileManager *fm, const char *path);
FileOperationResult file_manager_mkdir(FileManager *fm, const char *name);
FileOperationResult file_manager_rename(FileManager *fm, const char *old_name, const char *new_name);

// 文件信息函数
FileInfo *file_manager_get_selected_files(FileManager *fm);
const char *file_manager_get_current_path(FileManager *fm);
int file_manager_get_file_count(FileManager *fm);

// 过滤和搜索
void file_manager_set_filter(FileManager *fm, const char *pattern);
void file_manager_toggle_hidden_files(FileManager *fm);

// TUI 对话框函数
void show_file_manager_dialog(void);
void show_file_info_dialog(const char *file_path);
void show_copy_dialog(FileManager *fm);
void show_move_dialog(FileManager *fm);
void show_delete_confirmation_dialog(const char *file_path);
void show_rename_dialog(const char *old_name, char *new_name, size_t size);

#endif