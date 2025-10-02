#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include "layout_manager.h"
#include "logger.h"

// 初始化布局管理器
int layout_manager_init(LayoutManager *lm, const char *layout_dir) {
    if (!lm || !layout_dir) {
        return SWK_ERROR_INVALID_PARAM;
    }

    memset(lm, 0, sizeof(LayoutManager));
    strncpy(lm->layout_dir, layout_dir, sizeof(lm->layout_dir) - 1);

    // 检测终端能力
    if (layout_manager_detect_terminal(lm) != SWK_SUCCESS) {
        log_message(LOG_WARNING, "Failed to detect terminal capabilities");
    }

    // 创建布局目录（如果不存在）
    struct stat st = {0};
    if (stat(layout_dir, &st) == -1) {
        if (mkdir(layout_dir, 0755) != 0) {
            log_message(LOG_ERROR, "Failed to create layout directory: %s", layout_dir);
            return SWK_ERROR_SYSTEM_CALL;
        }
    }

    // 创建预设布局
    if (layout_manager_create_preset_layouts(lm) != SWK_SUCCESS) {
        log_message(LOG_WARNING, "Failed to create preset layouts");
    }

    // 加载用户布局
    if (layout_manager_load_layouts(lm) != SWK_SUCCESS) {
        log_message(LOG_WARNING, "Failed to load user layouts");
    }

    log_message(LOG_DEBUG, "Layout manager initialized with %d layouts", lm->layout_count);
    return SWK_SUCCESS;
}

// 清理布局管理器
void layout_manager_cleanup(LayoutManager *lm) {
    if (!lm) return;

    // 释放布局列表
    if (lm->layouts) {
        free(lm->layouts);
        lm->layouts = NULL;
    }

    lm->current_layout = NULL;
    lm->layout_count = 0;
}

// 检测终端能力
int layout_manager_detect_terminal(LayoutManager *lm) {
    if (!lm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 获取屏幕尺寸
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        lm->screen_width = ws.ws_col;
        lm->screen_height = ws.ws_row;
    } else {
        // 默认尺寸
        lm->screen_width = 80;
        lm->screen_height = 24;
    }

    // 检测颜色支持
    char *term = getenv("TERM");
    if (term) {
        if (strstr(term, "color") || strstr(term, "ansi")) {
            lm->color_support = 16; // 16色支持
        } else if (strstr(term, "256color")) {
            lm->color_support = 256; // 256色支持
        } else {
            lm->color_support = 8; // 基本颜色支持
        }
    } else {
        lm->color_support = 0; // 无颜色支持
    }

    // 检测终端类型
    if (strstr(term, "xterm") || strstr(term, "rxvt") || strstr(term, "screen")) {
        lm->terminal_type = 1; // 现代终端
    } else {
        lm->terminal_type = 0; // 传统终端
    }

    log_message(LOG_DEBUG, "Terminal: %dx%d, colors: %d, type: %d",
               lm->screen_width, lm->screen_height, lm->color_support, lm->terminal_type);
    return SWK_SUCCESS;
}

// 创建预设布局
int layout_manager_create_preset_layouts(LayoutManager *lm) {
    if (!lm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 默认紧凑布局
    LayoutConfig compact_layout;
    memset(&compact_layout, 0, sizeof(LayoutConfig));
    strcpy(compact_layout.name, "compact");
    strcpy(compact_layout.description, "紧凑布局，适合小屏幕");

    compact_layout.window_size.width = 80;
    compact_layout.window_size.height = 24;
    compact_layout.dialog_size.width = 60;
    compact_layout.dialog_size.height = 16;
    compact_layout.margins.left = 2;
    compact_layout.margins.right = 2;
    compact_layout.margins.top = 1;
    compact_layout.margins.bottom = 1;
    compact_layout.spacing = 1;
    compact_layout.compact_mode = 1;

    // 宽屏布局
    LayoutConfig widescreen_layout;
    memset(&widescreen_layout, 0, sizeof(LayoutConfig));
    strcpy(widescreen_layout.name, "widescreen");
    strcpy(widescreen_layout.description, "宽屏布局，适合大屏幕");

    widescreen_layout.window_size.width = 120;
    widescreen_layout.window_size.height = 30;
    widescreen_layout.dialog_size.width = 90;
    widescreen_layout.dialog_size.height = 20;
    widescreen_layout.margins.left = 3;
    widescreen_layout.margins.right = 3;
    widescreen_layout.margins.top = 2;
    widescreen_layout.margins.bottom = 2;
    widescreen_layout.spacing = 2;

    // 移动端布局
    LayoutConfig mobile_layout;
    memset(&mobile_layout, 0, sizeof(LayoutConfig));
    strcpy(mobile_layout.name, "mobile");
    strcpy(mobile_layout.description, "移动端优化布局");

    mobile_layout.window_size.width = 60;
    mobile_layout.window_size.height = 20;
    mobile_layout.dialog_size.width = 50;
    mobile_layout.dialog_size.height = 14;
    mobile_layout.margins.left = 1;
    mobile_layout.margins.right = 1;
    mobile_layout.margins.top = 1;
    mobile_layout.margins.bottom = 1;
    mobile_layout.spacing = 1;
    mobile_layout.mobile_optimized = 1;

    // 保存预设布局
    lm->layouts = malloc(sizeof(LayoutConfig) * 3);
    if (!lm->layouts) {
        return SWK_ERROR_OUT_OF_MEMORY;
    }

    memcpy(&lm->layouts[0], &compact_layout, sizeof(LayoutConfig));
    memcpy(&lm->layouts[1], &widescreen_layout, sizeof(LayoutConfig));
    memcpy(&lm->layouts[2], &mobile_layout, sizeof(LayoutConfig));

    lm->layout_count = 3;
    lm->current_layout = &lm->layouts[0]; // 默认使用紧凑布局

    log_message(LOG_DEBUG, "Created %d preset layouts", lm->layout_count);
    return SWK_SUCCESS;
}

// 应用布局
int layout_manager_apply_layout(LayoutManager *lm, const char *layout_name) {
    if (!lm || !layout_name) {
        return SWK_ERROR_INVALID_PARAM;
    }

    for (int i = 0; i < lm->layout_count; i++) {
        if (strcmp(lm->layouts[i].name, layout_name) == 0) {
            lm->current_layout = &lm->layouts[i];

            // 应用布局到当前会话
            layout_manager_apply_to_dialog(lm, NULL);

            log_message(LOG_INFO, "Applied layout: %s", layout_name);
            return SWK_SUCCESS;
        }
    }

    return SWK_ERROR_FILE_NOT_FOUND;
}

// 自适应屏幕尺寸
int layout_manager_adapt_to_screen(LayoutManager *lm, int width, int height) {
    if (!lm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    lm->screen_width = width;
    lm->screen_height = height;

    if (!lm->current_layout) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 根据屏幕尺寸调整布局
    if (lm->current_layout->responsive_design) {
        if (width < 80) {
            // 小屏幕优化
            lm->current_layout->dialog_size.width = width - 10;
            lm->current_layout->dialog_size.height = height - 6;
        } else if (width > 120) {
            // 大屏幕优化
            lm->current_layout->dialog_size.width = 90;
            lm->current_layout->dialog_size.height = 25;
        } else {
            // 标准屏幕
            lm->current_layout->dialog_size.width = 70;
            lm->current_layout->dialog_size.height = 20;
        }
    }

    return SWK_SUCCESS;
}

// 计算最优尺寸
int layout_manager_calculate_optimal_size(LayoutManager *lm, LayoutDimensions *content, LayoutDimensions *optimal) {
    if (!lm || !content || !optimal) {
        return SWK_ERROR_INVALID_PARAM;
    }

    if (!lm->current_layout) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 基于内容和布局配置计算最优尺寸
    optimal->width = content->width + lm->current_layout->margins.left + lm->current_layout->margins.right;
    optimal->height = content->height + lm->current_layout->margins.top + lm->current_layout->margins.bottom;

    // 确保尺寸在合理范围内
    if (optimal->width < lm->current_layout->dialog_size.min_width) {
        optimal->width = lm->current_layout->dialog_size.min_width;
    }
    if (optimal->width > lm->current_layout->dialog_size.max_width) {
        optimal->width = lm->current_layout->dialog_size.max_width;
    }

    if (optimal->height < lm->current_layout->dialog_size.min_height) {
        optimal->height = lm->current_layout->dialog_size.min_height;
    }
    if (optimal->height > lm->current_layout->dialog_size.max_height) {
        optimal->height = lm->current_layout->dialog_size.max_height;
    }

    return SWK_SUCCESS;
}

// 居中窗口
int layout_manager_center_window(LayoutManager *lm, LayoutDimensions *window) {
    if (!lm || !window) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 在屏幕上居中窗口
    window->x = (lm->screen_width - window->width) / 2;
    window->y = (lm->screen_height - window->height) / 2;

    return SWK_SUCCESS;
}

// 应用布局到对话框
int layout_manager_apply_to_dialog(LayoutManager *lm, const char *dialog_name) {
    if (!lm) {
        return SWK_ERROR_INVALID_PARAM;
    }

    if (!lm->current_layout) {
        return SWK_ERROR_INVALID_PARAM;
    }

    // 这里可以实现将布局应用到dialog库的逻辑
    // 由于dialog库的限制，我们主要通过全局变量来影响布局

    log_message(LOG_DEBUG, "Applied layout '%s' to dialogs", lm->current_layout->name);
    return SWK_SUCCESS;
}

// 初始化布局尺寸
void layout_dimensions_init(LayoutDimensions *dim, int width, int height) {
    if (!dim) return;

    memset(dim, 0, sizeof(LayoutDimensions));
    dim->width = width;
    dim->height = height;
    dim->min_width = width;
    dim->min_height = height;
    dim->max_width = width * 2;
    dim->max_height = height * 2;
}

// 初始化布局边距
void layout_margins_init(LayoutMargins *margins, int left, int right, int top, int bottom) {
    if (!margins) return;

    memset(margins, 0, sizeof(LayoutMargins));
    margins->left = left;
    margins->right = right;
    margins->top = top;
    margins->bottom = bottom;
}

// 计算内容区域
int layout_calculate_content_area(LayoutDimensions *window, LayoutMargins *margins, LayoutDimensions *content) {
    if (!window || !margins || !content) {
        return SWK_ERROR_INVALID_PARAM;
    }

    content->width = window->width - margins->left - margins->right;
    content->height = window->height - margins->top - margins->bottom;

    return SWK_SUCCESS;
}

// 检查是否为响应式布局
int layout_is_responsive(LayoutConfig *layout) {
    if (!layout) return 0;

    return layout->responsive_design || layout->adaptive_scaling || layout->mobile_optimized;
}

// 获取屏幕尺寸
int get_screen_dimensions(int *width, int *height) {
    if (!width || !height) {
        return SWK_ERROR_INVALID_PARAM;
    }

    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        *width = ws.ws_col;
        *height = ws.ws_row;
        return SWK_SUCCESS;
    }

    // 默认尺寸
    *width = 80;
    *height = 24;
    return SWK_SUCCESS;
}

// 获取最优对话框尺寸
int get_optimal_dialog_size(int content_lines, int content_width) {
    int screen_width, screen_height;

    if (get_screen_dimensions(&screen_width, &screen_height) != SWK_SUCCESS) {
        return SWK_ERROR;
    }

    // 基于内容和屏幕尺寸计算最优大小
    int optimal_width = content_width + 10;
    int optimal_height = content_lines + 8;

    // 确保尺寸在屏幕范围内
    if (optimal_width > screen_width - 4) {
        optimal_width = screen_width - 4;
    }
    if (optimal_height > screen_height - 2) {
        optimal_height = screen_height - 2;
    }

    // 设置全局对话框尺寸（这里只是示例，实际需要通过dialog库的API）
    // dialog_vars.default_width = optimal_width;
    // dialog_vars.default_height = optimal_height;

    return SWK_SUCCESS;
}