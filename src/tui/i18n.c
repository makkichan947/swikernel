#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>
#include <dirent.h>
#include <sys/stat.h>
#include "i18n.h"
#include "logger.h"

// 全局国际化系统实例
I18nSystem g_i18n_system;

// 语言信息数组
static LanguageInfo g_language_infos[] = {
    {LANG_CODE_EN, "English", "English", "🇺🇸"},
    {LANG_CODE_ZH_CN, "Simplified Chinese", "简体中文", "🇨🇳"},
    {LANG_CODE_ZH_TW, "Traditional Chinese", "繁體中文", "🇹🇼"},
    {LANG_CODE_JA, "Japanese", "日本語", "🇯🇵"},
    {LANG_CODE_KO, "Korean", "한국어", "🇰🇷"},
    {LANG_CODE_ES, "Spanish", "Español", "🇪🇸"},
    {LANG_CODE_FR, "French", "Français", "🇫🇷"},
    {LANG_CODE_DE, "German", "Deutsch", "🇩🇪"},
    {LANG_CODE_RU, "Russian", "Русский", "🇷🇺"},
    {LANG_CODE_PT, "Portuguese", "Português", "🇵🇹"}
};

// 初始化国际化系统
int i18n_init(I18nSystem *i18n, const char *locale_dir) {
    if (!i18n || !locale_dir) {
        return SWK_ERROR_INVALID_PARAM;
    }

    memset(i18n, 0, sizeof(I18nSystem));
    strncpy(i18n->locale_dir, locale_dir, sizeof(i18n->locale_dir) - 1);
    i18n->current_language = LANG_EN; // 默认英语
    i18n->fallback_to_english = 1;

    // 创建语言文件目录
    struct stat st = {0};
    if (stat(locale_dir, &st) == -1) {
        if (mkdir(locale_dir, 0755) != 0) {
            log_message(LOG_ERROR, "Failed to create locale directory: %s", locale_dir);
            return SWK_ERROR_SYSTEM_CALL;
        }
    }

    // 检测系统语言
    Language system_lang = i18n_detect_system_language();
    if (system_lang != LANG_EN) {
        i18n->current_language = system_lang;
    }

    // 加载当前语言的消息
    if (i18n_load_language(i18n, i18n->current_language) != SWK_SUCCESS) {
        log_message(LOG_WARNING, "Failed to load language messages, using English fallback");
        i18n->current_language = LANG_EN;
        i18n_load_language(i18n, LANG_EN);
    }

    log_message(LOG_DEBUG, "I18n system initialized (language: %s)", g_language_infos[i18n->current_language].code);
    return SWK_SUCCESS;
}

// 清理国际化系统
void i18n_cleanup(I18nSystem *i18n) {
    if (!i18n) return;

    if (i18n->messages) {
        free(i18n->messages);
        i18n->messages = NULL;
    }

    i18n->message_count = 0;
    log_message(LOG_DEBUG, "I18n system cleaned up");
}

// 设置语言
int i18n_set_language(I18nSystem *i18n, Language language) {
    if (!i18n) {
        return SWK_ERROR_INVALID_PARAM;
    }

    if (language >= LANG_COUNT) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 保存当前消息
    I18nMessage *old_messages = i18n->messages;
    int old_count = i18n->message_count;

    // 加载新语言
    if (i18n_load_language(i18n, language) != SWK_SUCCESS) {
        // 加载失败，回退到原语言
        i18n->messages = old_messages;
        i18n->message_count = old_count;
        return SWK_ERROR;
    }

    // 释放旧消息
    if (old_messages) {
        free(old_messages);
    }

    i18n->current_language = language;
    log_message(LOG_INFO, "Language changed to: %s", g_language_infos[language].code);
    return SWK_SUCCESS;
}

// 加载语言消息
int i18n_load_language(I18nSystem *i18n, Language language) {
    if (!i18n || language >= LANG_COUNT) {
        return SWK_ERROR_INVALID_PARAM;
    }

    char file_path[MAX_PATH_LENGTH];
    snprintf(file_path, sizeof(file_path), "%s/%s.po", i18n->locale_dir, g_language_infos[language].code);

    return i18n_load_messages(i18n, file_path);
}

// 加载消息文件
int i18n_load_messages(I18nSystem *i18n, const char *file_path) {
    if (!i18n || !file_path) {
        return SWK_ERROR_INVALID_PARAM;
    }

    FILE *file = fopen(file_path, "r");
    if (!file) {
        // 文件不存在，使用默认英语消息
        return i18n_load_default_messages(i18n);
    }

    // 清空现有消息
    if (i18n->messages) {
        free(i18n->messages);
        i18n->messages = NULL;
    }

    i18n->message_count = 0;

    char line[1024];
    char current_key[128] = {0};
    char current_text[512] = {0};
    int in_msgid = 0;
    int in_msgstr = 0;

    while (fgets(line, sizeof(line), file)) {
        // 移除换行符
        line[strcspn(line, "\n\r")] = '\0';

        // 跳过空行和注释
        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }

        if (strncmp(line, "msgid \"", 7) == 0) {
            // 开始新的msgid
            if (strlen(current_key) > 0) {
                // 保存上一条消息
                i18n_add_message(i18n, current_key, current_text);
            }

            // 解析新的msgid
            char *end_quote = strrchr(line + 7, '"');
            if (end_quote) {
                *end_quote = '\0';
                strncpy(current_key, line + 7, sizeof(current_key) - 1);
            }
            in_msgid = 1;
            in_msgstr = 0;
            current_text[0] = '\0';
        } else if (strncmp(line, "msgstr \"", 8) == 0) {
            // 开始msgstr
            char *end_quote = strrchr(line + 8, '"');
            if (end_quote) {
                *end_quote = '\0';
                strncpy(current_text, line + 8, sizeof(current_text) - 1);
            }
            in_msgid = 0;
            in_msgstr = 1;
        } else if (line[0] == '"' && (in_msgid || in_msgstr)) {
            // 多行字符串
            char *end_quote = strrchr(line + 1, '"');
            if (end_quote) {
                *end_quote = '\0';
                if (in_msgid) {
                    strncat(current_key, line + 1, sizeof(current_key) - strlen(current_key) - 1);
                } else if (in_msgstr) {
                    strncat(current_text, line + 1, sizeof(current_text) - strlen(current_text) - 1);
                }
            }
        }
    }

    // 保存最后一条消息
    if (strlen(current_key) > 0) {
        i18n_add_message(i18n, current_key, current_text);
    }

    fclose(file);
    log_message(LOG_DEBUG, "Loaded %d messages from %s", i18n->message_count, file_path);
    return SWK_SUCCESS;
}

// 添加消息到数组
int i18n_add_message(I18nSystem *i18n, const char *key, const char *text) {
    if (!i18n || !key || !text) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 重新分配消息数组
    I18nMessage *new_messages = realloc(i18n->messages, sizeof(I18nMessage) * (i18n->message_count + 1));
    if (!new_messages) {
        return SWK_ERROR_OUT_OF_MEMORY;
    }

    i18n->messages = new_messages;

    // 添加新消息
    strncpy(i18n->messages[i18n->message_count].key, key, sizeof(i18n->messages[i18n->message_count].key) - 1);
    strncpy(i18n->messages[i18n->message_count].text, text, sizeof(i18n->messages[i18n->message_count].text) - 1);

    i18n->message_count++;
    return SWK_SUCCESS;
}

// 加载默认英语消息
int i18n_load_default_messages(I18nSystem *i18n) {
    if (!i18n) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 清空现有消息
    if (i18n->messages) {
        free(i18n->messages);
        i18n->messages = NULL;
    }
    i18n->message_count = 0;

    // 添加基本消息
    i18n_add_message(i18n, "app_name", "SwiKernel");
    i18n_add_message(i18n, "app_description", "Linux Kernel Switcher");
    i18n_add_message(i18n, "main_menu_title", "Main Menu");
    i18n_add_message(i18n, "select_option", "Select an option:");
    i18n_add_message(i18n, "exit", "Exit");
    i18n_add_message(i18n, "back", "Back");
    i18n_add_message(i18n, "cancel", "Cancel");
    i18n_add_message(i18n, "confirm", "Confirm");
    i18n_add_message(i18n, "yes", "Yes");
    i18n_add_message(i18n, "no", "No");
    i18n_add_message(i18n, "ok", "OK");
    i18n_add_message(i18n, "error", "Error");
    i18n_add_message(i18n, "warning", "Warning");
    i18n_add_message(i18n, "info", "Information");
    i18n_add_message(i18n, "success", "Success");
    i18n_add_message(i18n, "loading", "Loading...");
    i18n_add_message(i18n, "processing", "Processing...");
    i18n_add_message(i18n, "completed", "Completed");
    i18n_add_message(i18n, "failed", "Failed");

    // 菜单项
    i18n_add_message(i18n, "menu_kernel_management", "Kernel Management");
    i18n_add_message(i18n, "menu_install_kernel", "Install New Kernel");
    i18n_add_message(i18n, "menu_switch_kernel", "Switch Active Kernel");
    i18n_add_message(i18n, "menu_kernel_sources", "Kernel Sources");
    i18n_add_message(i18n, "menu_file_manager", "File Manager");
    i18n_add_message(i18n, "menu_log_viewer", "Log Viewer");
    i18n_add_message(i18n, "menu_configuration", "Configuration");
    i18n_add_message(i18n, "menu_plugin_manager", "Plugin Manager");
    i18n_add_message(i18n, "menu_system_monitor", "System Monitor");
    i18n_add_message(i18n, "menu_theme_manager", "Theme Manager");
    i18n_add_message(i18n, "menu_layout_manager", "Layout Manager");

    log_message(LOG_DEBUG, "Loaded %d default English messages", i18n->message_count);
    return SWK_SUCCESS;
}

// 获取翻译文本
const char *i18n_get_text(I18nSystem *i18n, const char *key) {
    if (!i18n || !key) return key;

    // 在消息数组中查找
    for (int i = 0; i < i18n->message_count; i++) {
        if (strcmp(i18n->messages[i].key, key) == 0) {
            // 如果翻译文本为空，返回键值
            if (strlen(i18n->messages[i].text) == 0) {
                return key;
            }
            return i18n->messages[i].text;
        }
    }

    // 未找到翻译，回退到英语或其他语言
    if (i18n->fallback_to_english && i18n->current_language != LANG_EN) {
        // 这里可以实现回退逻辑
        return key;
    }

    return key;
}

// 获取格式化翻译文本
const char *i18n_get_text_f(I18nSystem *i18n, const char *key, ...) {
    static char buffer[1024];
    const char *text = i18n_get_text(i18n, key);

    va_list args;
    va_start(args, key);
    vsnprintf(buffer, sizeof(buffer), text, args);
    va_end(args);

    return buffer;
}

// 获取语言信息
LanguageInfo *i18n_get_language_info(Language language) {
    if (language >= LANG_COUNT) {
        return &g_language_infos[LANG_EN];
    }
    return &g_language_infos[language];
}

// 根据代码获取语言
Language i18n_get_language_by_code(const char *code) {
    if (!code) return LANG_EN;

    for (int i = 0; i < LANG_COUNT; i++) {
        if (strcmp(g_language_infos[i].code, code) == 0) {
            return (Language)i;
        }
    }

    return LANG_EN;
}

// 获取语言代码
const char *i18n_get_language_code(Language language) {
    if (language >= LANG_COUNT) {
        return g_language_infos[LANG_EN].code;
    }
    return g_language_infos[language].code;
}

// 获取语言名称
const char *i18n_get_language_name(Language language) {
    if (language >= LANG_COUNT) {
        return g_language_infos[LANG_EN].name;
    }
    return g_language_infos[language].name;
}

// 检测系统语言
Language i18n_detect_system_language(void) {
    char *locale = getenv("LANG");
    if (!locale) {
        locale = getenv("LC_ALL");
    }
    if (!locale) {
        return LANG_EN;
    }

    // 解析语言代码
    char lang_code[8];
    strncpy(lang_code, locale, sizeof(lang_code) - 1);
    lang_code[sizeof(lang_code) - 1] = '\0';

    // 截取主要语言代码
    char *dot = strchr(lang_code, '.');
    if (dot) *dot = '\0';

    char *underscore = strchr(lang_code, '_');
    if (underscore) *underscore = '\0';

    return i18n_get_language_by_code(lang_code);
}

// 设置首次运行
int i18n_setup_first_run(I18nSystem *i18n) {
    if (!i18n) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 显示语言选择对话框
    show_first_run_language_dialog(i18n);

    // 保存语言设置
    char config_file[MAX_PATH_LENGTH];
    snprintf(config_file, sizeof(config_file), "%s/language.conf", i18n->locale_dir);

    FILE *file = fopen(config_file, "w");
    if (file) {
        fprintf(file, "language = %s\n", i18n_get_language_code(i18n->current_language));
        fclose(file);
    }

    return SWK_SUCCESS;
}

// 显示首次运行语言选择对话框
void show_first_run_language_dialog(I18nSystem *i18n) {
    if (!i18n) return;

    char display_text[MAX_BUFFER_SIZE] = {0};
    snprintf(display_text, sizeof(display_text),
            "Welcome to SwiKernel!\n"
            "欢迎使用 SwiKernel！\n\n"
            "Please select your language / 请选择您的语言:\n");

    // 构建语言菜单
    char menu_items[1024] = {0};
    for (int i = 0; i < LANG_COUNT; i++) {
        char item_num[8];
        snprintf(item_num, sizeof(item_num), "%d", i + 1);
        strncat(menu_items, item_num, sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, g_language_infos[i].native_name, sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, " ", sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, g_language_infos[i].flag, sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
    }

    int choice = dialog_menu("Language Selection / 语言选择",
                           display_text,
                           20, 80, LANG_COUNT,
                           menu_items);

    if (choice >= 1 && choice <= LANG_COUNT) {
        i18n_set_language(i18n, (Language)(choice - 1));
    }
}

// 显示语言选择对话框
void show_language_selector_dialog(I18nSystem *i18n) {
    if (!i18n) return;

    char display_text[MAX_BUFFER_SIZE] = {0};
    snprintf(display_text, sizeof(display_text),
            "Current Language: %s %s\n\n"
            "Available Languages:",
            g_language_infos[i18n->current_language].native_name,
            g_language_infos[i18n->current_language].flag);

    // 构建语言菜单
    char menu_items[1024] = {0};
    for (int i = 0; i < LANG_COUNT; i++) {
        char item_num[8];
        snprintf(item_num, sizeof(item_num), "%d", i + 1);
        strncat(menu_items, item_num, sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, g_language_infos[i].native_name, sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, " ", sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, g_language_infos[i].flag, sizeof(menu_items) - strlen(menu_items) - 1);
        strncat(menu_items, "\n", sizeof(menu_items) - strlen(menu_items) - 1);
    }

    int choice = dialog_menu("Language Selection",
                           display_text,
                           20, 80, LANG_COUNT,
                           menu_items);

    if (choice >= 1 && choice <= LANG_COUNT) {
        i18n_set_language(i18n, (Language)(choice - 1));
    }
}

// 创建语言模板
int i18n_create_language_template(I18nSystem *i18n, Language language) {
    if (!i18n || language >= LANG_COUNT) {
        return SWK_ERROR_INVALID_PARAM;
    }

    char file_path[MAX_PATH_LENGTH];
    snprintf(file_path, sizeof(file_path), "%s/%s.po", i18n->locale_dir, g_language_infos[language].code);

    FILE *file = fopen(file_path, "w");
    if (!file) {
        return SWK_ERROR_FILE_NOT_FOUND;
    }

    fprintf(file, "# SwiKernel Language File\n");
    fprintf(file, "# Language: %s (%s)\n\n", g_language_infos[language].name, g_language_infos[language].code);

    // 写入基本消息模板
    fprintf(file, "msgid \"\"\n");
    fprintf(file, "msgstr \"\"\n");
    fprintf(file, "\"Language: %s\\n\"\n", g_language_infos[language].native_name);
    fprintf(file, "\"Content-Type: text/plain; charset=UTF-8\\n\"\n\n");

    fprintf(file, "msgid \"app_name\"\n");
    fprintf(file, "msgstr \"SwiKernel\"\n\n");

    fprintf(file, "msgid \"app_description\"\n");
    fprintf(file, "msgstr \"Linux Kernel Switcher\"\n\n");

    fclose(file);
    log_message(LOG_INFO, "Created language template: %s", file_path);
    return SWK_SUCCESS;
}