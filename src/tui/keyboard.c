#include <ncurses.h>
#include <string.h>
#include "keyboard.h"
#include "logger.h"

static int keyboard_initialized = 0;

// 初始化键盘处理器
int init_keyboard_handler(void) {
    if (keyboard_initialized) {
        return 0;
    }

    // 启用键盘特殊键
    keypad(stdscr, TRUE);
    // 不回显输入字符
    noecho();
    // 设置非阻塞模式
    nodelay(stdscr, TRUE);
    // 隐藏光标
    curs_set(0);

    keyboard_initialized = 1;
    log_message(LOG_DEBUG, "Keyboard handler initialized");
    return 0;
}

// 清理键盘处理器
void cleanup_keyboard_handler(void) {
    if (keyboard_initialized) {
        // 恢复终端设置
        echo();
        nodelay(stdscr, FALSE);
        curs_set(1);
        keyboard_initialized = 0;
        log_message(LOG_DEBUG, "Keyboard handler cleaned up");
    }
}

// 处理键盘输入
int handle_key_input(int ch, void (*callback)(KeyBinding, void*), void* user_data) {
    if (!keyboard_initialized || !callback) {
        return -1;
    }

    KeyBinding key = KEY_ESCAPE; // 默认值

    switch (ch) {
        case KEY_HELP:
        case 'h':
        case 'H':
            key = KEY_HELP;
            break;
        case KEY_REFRESH:
        case 'r':
        case 'R':
            key = KEY_REFRESH;
            break;
        case KEY_SEARCH:
        case '/':
            key = KEY_SEARCH;
            break;
        case KEY_FILTER:
        case 'f':
        case 'F':
            key = KEY_FILTER;
            break;
        case KEY_SORT:
        case 's':
        case 'S':
            key = KEY_SORT;
            break;
        case KEY_INFO:
        case 'i':
        case 'I':
            key = KEY_INFO;
            break;
        case KEY_DELETE:
        case 'd':
        case 'D':
            key = KEY_DELETE;
            break;
        case KEY_EDIT:
        case 'e':
        case 'E':
            key = KEY_EDIT;
            break;
        case KEY_COPY:
        case 'c':
        case 'C':
            key = KEY_COPY;
            break;
        case KEY_MOVE:
        case 'm':
        case 'M':
            key = KEY_MOVE;
            break;
        case KEY_RENAME:
        case 'n':
        case 'N':
            key = KEY_RENAME;
            break;
        case KEY_MKDIR:
        case 'k':
        case 'K':
            key = KEY_MKDIR;
            break;
        case KEY_UP:
            key = KEY_UP;
            break;
        case KEY_DOWN:
            key = KEY_DOWN;
            break;
        case KEY_LEFT:
            key = KEY_LEFT;
            break;
        case KEY_RIGHT:
            key = KEY_RIGHT;
            break;
        case KEY_PAGE_UP:
            key = KEY_PAGE_UP;
            break;
        case KEY_PAGE_DOWN:
            key = KEY_PAGE_DOWN;
            break;
        case KEY_HOME:
            key = KEY_HOME;
            break;
        case KEY_END:
            key = KEY_END;
            break;
        case KEY_TAB:
            key = KEY_TAB;
            break;
        case KEY_ENTER:
        case '\n':
            key = KEY_ENTER;
            break;
        case KEY_BACKSPACE:
            key = KEY_BACKSPACE;
            break;
        case KEY_F(1):
            key = KEY_F1;
            break;
        case KEY_F(2):
            key = KEY_F2;
            break;
        case KEY_F(3):
            key = KEY_F3;
            break;
        case KEY_F(4):
            key = KEY_F4;
            break;
        case KEY_F(5):
            key = KEY_F5;
            break;
        case KEY_F(10):
            key = KEY_F10;
            break;
        case KEY_F(12):
            key = KEY_F12;
            break;
        case 27: // ESC
            key = KEY_ESCAPE;
            break;
        default:
            // 非快捷键，直接返回原字符
            return ch;
    }

    // 调用回调函数处理快捷键
    callback(key, user_data);
    return 0;
}

// 检查是否为快捷键
int is_shortcut_key(int ch) {
    switch (ch) {
        case 'h': case 'H':
        case 'r': case 'R':
        case '/':
        case 'f': case 'F':
        case 's': case 'S':
        case 'i': case 'I':
        case 'd': case 'D':
        case 'e': case 'E':
        case 'c': case 'C':
        case 'm': case 'M':
        case 'n': case 'N':
        case 'k': case 'K':
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        case KEY_PPAGE:
        case KEY_NPAGE:
        case KEY_HOME:
        case KEY_END:
        case '\t':
        case '\n':
        case KEY_BACKSPACE:
        case KEY_F(1):
        case KEY_F(2):
        case KEY_F(3):
        case KEY_F(4):
        case KEY_F(5):
        case KEY_F(10):
        case KEY_F(12):
        case 27: // ESC
            return 1;
        default:
            return 0;
    }
}

// 获取快捷键帮助信息
const char* get_key_binding_help(void) {
    return
        "快捷键帮助:\n"
        "===============\n"
        "导航:\n"
        "  ↑/↓/←/→     - 方向键导航\n"
        "  Page Up/Down - 翻页\n"
        "  Home/End     - 跳到开始/结束\n"
        "  Tab          - 切换焦点\n"
        "\n操作:\n"
        "  Enter        - 确认/进入\n"
        "  Esc          - 返回/取消\n"
        "  h/H          - 显示帮助\n"
        "  r/R          - 刷新\n"
        "  /            - 搜索\n"
        "  f/F          - 过滤\n"
        "  s/S          - 排序\n"
        "\n文件管理:\n"
        "  i/I          - 显示信息\n"
        "  d/D          - 删除\n"
        "  c/C          - 复制\n"
        "  m/M          - 移动\n"
        "  n/N          - 重命名\n"
        "  k/K          - 新建目录\n"
        "  e/E          - 编辑\n"
        "\n功能键:\n"
        "  F1           - 主菜单\n"
        "  F2           - 文件管理\n"
        "  F3           - 日志查看\n"
        "  F4           - 配置\n"
        "  F5           - 刷新\n"
        "  F10          - 设置\n"
        "  F12          - 关于\n";
}