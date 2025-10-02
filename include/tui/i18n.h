#ifndef I18N_H
#define I18N_H

#include "../common_defs.h"

// 支持的语言
typedef enum {
    LANG_EN,    // 英语
    LANG_ZH_CN, // 简体中文
    LANG_ZH_TW, // 繁体中文
    LANG_JA,    // 日语
    LANG_KO,    // 韩语
    LANG_ES,    // 西班牙语
    LANG_FR,    // 法语
    LANG_DE,    // 德语
    LANG_RU,    // 俄语
    LANG_PT,    // 葡萄牙语
    LANG_COUNT  // 语言总数
} Language;

// 语言信息结构
typedef struct {
    char code[8];        // 语言代码，如 "en", "zh_CN"
    char name[32];       // 语言名称，如 "English", "简体中文"
    char native_name[32]; // 本地名称，如 "English", "简体中文"
    char flag[8];        // 国旗表情符号
} LanguageInfo;

// 国际化消息结构
typedef struct {
    char key[128];       // 消息键
    char text[512];      // 消息文本
} I18nMessage;

// 国际化系统状态
typedef struct {
    Language current_language;     // 当前语言
    I18nMessage *messages;         // 消息数组
    int message_count;             // 消息数量
    char locale_dir[MAX_PATH_LENGTH]; // 语言文件目录
    int fallback_to_english;       // 是否回退到英语
} I18nSystem;

// 国际化系统函数
int i18n_init(I18nSystem *i18n, const char *locale_dir);
void i18n_cleanup(I18nSystem *i18n);
int i18n_set_language(I18nSystem *i18n, Language language);
Language i18n_get_language(I18nSystem *i18n);
int i18n_load_language(I18nSystem *i18n, Language language);

// 消息翻译函数
const char *i18n_get_text(I18nSystem *i18n, const char *key);
const char *i18n_get_text_f(I18nSystem *i18n, const char *key, ...);

// 语言信息函数
LanguageInfo *i18n_get_language_info(Language language);
Language i18n_get_language_by_code(const char *code);
const char *i18n_get_language_code(Language language);
const char *i18n_get_language_name(Language language);

// 语言文件管理
int i18n_load_messages(I18nSystem *i18n, const char *file_path);
int i18n_save_messages(I18nSystem *i18n, Language language, const char *file_path);
int i18n_create_language_template(I18nSystem *i18n, Language language);

// 自动检测和设置
Language i18n_detect_system_language(void);
int i18n_setup_first_run(I18nSystem *i18n);

// TUI 语言选择对话框
void show_language_selector_dialog(I18nSystem *i18n);
void show_first_run_language_dialog(I18nSystem *i18n);

// 便捷宏定义
#define _(key) i18n_get_text(&g_i18n_system, key)
#define _f(key, ...) i18n_get_text_f(&g_i18n_system, key, __VA_ARGS__)

// 全局国际化系统实例
extern I18nSystem g_i18n_system;

// 语言代码常量
#define LANG_CODE_EN "en"
#define LANG_CODE_ZH_CN "zh_CN"
#define LANG_CODE_ZH_TW "zh_TW"
#define LANG_CODE_JA "ja"
#define LANG_CODE_KO "ko"
#define LANG_CODE_ES "es"
#define LANG_CODE_FR "fr"
#define LANG_CODE_DE "de"
#define LANG_CODE_RU "ru"
#define LANG_CODE_PT "pt"

#endif