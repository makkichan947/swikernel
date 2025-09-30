#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include "../common_defs.h"

#define MAX_MATCHES 100

// 自动补全类型
typedef enum {
    AUTOCOMPLETE_PATH,
    AUTOCOMPLETE_KERNEL,
    AUTOCOMPLETE_COMMAND
} AutocompleteType;

// 自动补全函数
char** path_autocomplete(const char *partial_path, int *match_count);
char** kernel_name_autocomplete(const char *partial_name, int *match_count);
void setup_autocomplete_dialog(const char *title, const char *prompt, 
                              AutocompleteType type, char *result, size_t result_size);
int show_autocomplete_menu(char **matches, int match_count);

#endif