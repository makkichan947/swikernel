#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "../swikernel.h"

// 配置管理函数
int load_config(SwikernelConfig *config);
int save_config(const SwikernelConfig *config, const char *filename);
void set_default_config(SwikernelConfig *config);
int set_config_value(SwikernelConfig *config, const char *section, const char *key, const char *value);
char* trim_whitespace(char *str);
int load_config_from_file(SwikernelConfig *config, const char *filename);

#endif