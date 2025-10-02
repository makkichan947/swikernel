#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <ncurses.h>

// 快捷键定义
typedef enum {
    KEY_HELP = 'h',
    KEY_REFRESH = 'r',
    KEY_SEARCH = '/',
    KEY_FILTER = 'f',
    KEY_SORT = 's',
    KEY_INFO = 'i',
    KEY_DELETE = 'd',
    KEY_EDIT = 'e',
    KEY_COPY = 'c',
    KEY_MOVE = 'm',
    KEY_RENAME = 'n',
    KEY_MKDIR = 'k',
    KEY_UP = KEY_UP,
    KEY_DOWN = KEY_DOWN,
    KEY_LEFT = KEY_LEFT,
    KEY_RIGHT = KEY_RIGHT,
    KEY_PAGE_UP = KEY_PPAGE,
    KEY_PAGE_DOWN = KEY_NPAGE,
    KEY_HOME = KEY_HOME,
    KEY_END = KEY_END,
    KEY_TAB = '\t',
    KEY_ESCAPE = 27,
    KEY_ENTER = '\n',
    KEY_BACKSPACE = KEY_BACKSPACE,
    KEY_F1 = KEY_F(1),
    KEY_F2 = KEY_F(2),
    KEY_F3 = KEY_F(3),
    KEY_F4 = KEY_F(4),
    KEY_F5 = KEY_F(5),
    KEY_F10 = KEY_F(10),
    KEY_F12 = KEY_F(12)
} KeyBinding;

// 键盘处理函数
int init_keyboard_handler(void);
void cleanup_keyboard_handler(void);
int handle_key_input(int ch, void (*callback)(KeyBinding, void*), void* user_data);
int is_shortcut_key(int ch);

// 快捷键帮助信息
const char* get_key_binding_help(void);

#endif