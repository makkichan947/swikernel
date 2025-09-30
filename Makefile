# SwiKernel Makefile
# 支持并行编译和增量构建

# 版本信息
VERSION = 1.0.0
BUILD_DATE = $(shell date +%Y%m%d)

# 工具链配置
CC = gcc
ASM = nasm
LD = ld
AR = ar

# 编译标志
CFLAGS = -Wall -Wextra -O2 -g -I./include -I./src
CFLAGS_DEBUG = -g -O0 -DDEBUG -fsanitize=address
CFLAGS_RELEASE = -O2 -DNDEBUG -flto
CFLAGS_WARNING = -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes

# 链接标志
LDFLAGS = -lm -ldl -lpthread
LDFLAGS_DEBUG = -fsanitize=address
LDFLAGS_RELEASE = -flto

# 目录配置
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
LIB_DIR = lib
INCLUDE_DIR = include

# 源文件配置
C_SOURCES = $(shell find $(SRC_DIR) -name '*.c')
ASM_SOURCES = $(shell find $(SRC_DIR) -name '*.asm')

# 对象文件配置
C_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SOURCES))
ASM_OBJECTS = $(patsubst $(SRC_DIR)/%.asm,$(OBJ_DIR)/%.o,$(ASM_SOURCES))
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# 依赖文件
DEPENDS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.d,$(C_SOURCES))

# 目标配置
TARGET = $(BIN_DIR)/swikernel
STATIC_LIB = $(LIB_DIR)/libswikernel.a

# 包含目录
INCLUDES = -I$(INCLUDE_DIR) -I$(SRC_DIR)

# 外部库
LIBS = -ldialog -lncurses -lm

# 构建模式
BUILD_MODE ?= RELEASE

# 根据构建模式设置标志
ifeq ($(BUILD_MODE), DEBUG)
    CFLAGS += $(CFLAGS_DEBUG) $(CFLAGS_WARNING)
    LDFLAGS += $(LDFLAGS_DEBUG)
else
    CFLAGS += $(CFLAGS_RELEASE)
    LDFLAGS += $(LDFLAGS_RELEASE)
endif

# 默认目标
.PHONY: all
all: $(TARGET) static_lib

# 创建目录
$(OBJ_DIR) $(BIN_DIR) $(LIB_DIR):
	@mkdir -p $@

# 主目标
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@
	@echo "Build completed: $(TARGET)"

# 静态库
static_lib: $(STATIC_LIB)

$(STATIC_LIB): $(OBJECTS) | $(LIB_DIR)
	@echo "Creating static library $(STATIC_LIB)..."
	$(AR) rcs $@ $(OBJECTS)

# C源文件编译规则
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 汇编文件编译规则
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@echo "Assembling $<..."
	$(ASM) -f elf64 $< -o $@

# 依赖生成
$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -MM -MT $(@:.d=.o) $< > $@

# 包含依赖
-include $(DEPENDS)

# 安装目标
.PHONY: install
install: $(TARGET)
	@echo "Installing SwiKernel..."
	install -d /usr/local/bin
	install -m 755 $(TARGET) /usr/local/bin/swikernel
	install -d /etc/swikernel
	install -m 644 config/swikernel.conf /etc/swikernel/
	install -d /usr/local/share/swikernel/scripts
	install -m 755 scripts/*.sh /usr/local/share/swikernel/scripts/
	@echo "Installation completed"

# 卸载目标
.PHONY: uninstall
uninstall:
	@echo "Uninstalling SwiKernel..."
	rm -f /usr/local/bin/swikernel
	rm -rf /etc/swikernel
	rm -rf /usr/local/share/swikernel
	@echo "Uninstallation completed"

# 测试目标
.PHONY: test
test: all
	@echo "Running tests..."
	$(MAKE) -C tests all
	@echo "All tests passed"

# 清理目标
.PHONY: clean
clean:
	@echo "Cleaning build files..."
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
	$(MAKE) -C tests clean
	@echo "Clean completed"

.PHONY: distclean
distclean: clean
	@echo "Performing deep clean..."
	rm -f *.log *.tmp
	find . -name "*.o" -delete
	find . -name "*.d" -delete
	find . -name "*.a" -delete
	@echo "Distclean completed"

# 调试构建
.PHONY: debug
debug:
	$(MAKE) BUILD_MODE=DEBUG

# 发布构建
.PHONY: release
release:
	$(MAKE) BUILD_MODE=RELEASE

# 依赖安装
.PHONY: deps
deps:
	@echo "Installing dependencies..."
	./scripts/setup_deps.sh

# 格式检查
.PHONY: format
format:
	@echo "Formatting code..."
	find $(SRC_DIR) -name '*.c' -exec clang-format -i {} \;
	find $(INCLUDE_DIR) -name '*.h' -exec clang-format -i {} \;

# 静态分析
.PHONY: analyze
analyze:
	@echo "Running static analysis..."
	find $(SRC_DIR) -name '*.c' -exec splint -I$(INCLUDE_DIR) {} \;

# 代码统计
.PHONY: stats
stats:
	@echo "Code statistics:"
	@echo "C source files: $$(find $(SRC_DIR) -name '*.c' | wc -l)"
	@echo "Assembly files: $$(find $(SRC_DIR) -name '*.asm' | wc -l)"
	@echo "Header files: $$(find $(INCLUDE_DIR) -name '*.h' | wc -l)"
	@echo "Total lines: $$(find $(SRC_DIR) $(INCLUDE_DIR) -name '*.[ch]' -exec cat {} \; | wc -l)"

# 帮助信息
.PHONY: help
help:
	@echo "SwiKernel Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build everything (default)"
	@echo "  debug        - Build with debug flags"
	@echo "  release      - Build with release flags"
	@echo "  install      - Install to system"
	@echo "  uninstall    - Uninstall from system"
	@echo "  test         - Run tests"
	@echo "  clean        - Clean build files"
	@echo "  distclean    - Deep clean"
	@echo "  deps         - Install dependencies"
	@echo "  format       - Format source code"
	@echo "  analyze      - Static code analysis"
	@echo "  stats        - Code statistics"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_MODE   - Build mode (DEBUG or RELEASE)"
	@echo "  CC           - C compiler"
	@echo "  ASM          - Assembler"

# 显示版本信息
.PHONY: version
version:
	@echo "SwiKernel version $(VERSION)"
	@echo "Build date: $(BUILD_DATE)"