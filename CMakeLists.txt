# SwiKernel CMake Build Configuration
# 支持跨平台构建

cmake_minimum_required(VERSION 3.12)
project(swikernel VERSION 1.0.0 LANGUAGES C)

# 项目信息
set(PROJECT_DESCRIPTION "Linux Kernel Switcher")
set(PROJECT_AUTHOR "Yaku Makki")
set(PROJECT_COPYRIGHT "2025 Yaku Makki")

# 编译选项
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 构建类型设置
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 编译标志
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")

# 平台特定设置
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    add_definitions(-D_POSIX_C_SOURCE=200809L)
endif()

# 目录配置
set(SOURCE_DIR src)
set(INCLUDE_DIR include)
set(TEST_DIR tests)
set(SCRIPT_DIR scripts)
set(CONFIG_DIR config)

# 包含目录
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})

# 查找依赖包
find_package(PkgConfig REQUIRED)

# 查找必要库
pkg_check_modules(DIALOG REQUIRED dialog)
pkg_check_modules(NCURSES REQUIRED ncurses)

# 查找汇编器
find_program(NASM_EXECUTABLE nasm)
if(NASM_EXECUTABLE)
    set(HAVE_NASM TRUE)
    message(STATUS "Found NASM: ${NASM_EXECUTABLE}")
else()
    set(HAVE_NASM FALSE)
    message(WARNING "NASM not found, assembly optimizations disabled")
endif()

# 源文件分组
set(CORE_SOURCES
    ${SOURCE_DIR}/core/main.c
    ${SOURCE_DIR}/core/swikernel.c
    ${SOURCE_DIR}/core/version.c
)

set(TUI_SOURCES
    ${SOURCE_DIR}/tui/tui_core.c
    ${SOURCE_DIR}/tui/tui_dialogs.c
    ${SOURCE_DIR}/tui/autocomplete.c
    ${SOURCE_DIR}/tui/progress_ui.c
)

set(KERNEL_SOURCES
    ${SOURCE_DIR}/kernel/kernel_manager.c
    ${SOURCE_DIR}/kernel/kernel_install.c
    ${SOURCE_DIR}/kernel/kernel_rollback.c
    ${SOURCE_DIR}/kernel/dependency_check.c
    ${SOURCE_DIR}/kernel/rolling_update.c
)

set(SYSTEM_SOURCES
    ${SOURCE_DIR}/system/file_ops.c
    ${SOURCE_DIR}/system/process.c
    ${SOURCE_DIR}/system/system_info.c
)

set(UTILS_SOURCES
    ${SOURCE_DIR}/utils/logger.c
    ${SOURCE_DIR}/utils/error_handler.c
    ${SOURCE_DIR}/utils/config_parser.c
    ${SOURCE_DIR}/utils/progress_bar.c
    ${SOURCE_DIR}/utils/security.c
)

# 汇编源文件 (如果可用)
if(HAVE_NASM)
    set(ASM_SOURCES
        ${SOURCE_DIR}/asm_optimized/string_ops.asm
        ${SOURCE_DIR}/asm_optimized/memory_ops.asm
        ${SOURCE_DIR}/asm_optimized/crypto_ops.asm
        ${SOURCE_DIR}/system/syscalls.asm
    )
    
    # 设置汇编编译规则
    enable_language(ASM_NASM)
    set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
    set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -f elf64")
else()
    set(ASM_SOURCES)
endif()

# 所有源文件
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${TUI_SOURCES}
    ${KERNEL_SOURCES}
    ${SYSTEM_SOURCES}
    ${UTILS_SOURCES}
    ${ASM_SOURCES}
)

# 创建可执行文件
add_executable(swikernel ${ALL_SOURCES})

# 设置目标属性
set_target_properties(swikernel PROPERTIES
    OUTPUT_NAME swikernel
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# 链接库
target_link_libraries(swikernel
    ${DIALOG_LIBRARIES}
    ${NCURSES_LIBRARIES}
    m
    dl
    pthread
)

# 包含目录
target_include_directories(swikernel PRIVATE
    ${DIALOG_INCLUDE_DIRS}
    ${NCURSES_INCLUDE_DIRS}
)

# 编译定义
target_compile_definitions(swikernel PRIVATE
    PROJECT_VERSION="${PROJECT_VERSION}"
    PROJECT_NAME="${PROJECT_NAME}"
)

# 创建静态库
add_library(swikernel_static STATIC ${ALL_SOURCES})
set_target_properties(swikernel_static PROPERTIES
    OUTPUT_NAME swikernel
    POSITION_INDEPENDENT_CODE ON
)

# 安装目标
install(TARGETS swikernel swikernel_static
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装头文件
install(DIRECTORY ${INCLUDE_DIR}/
    DESTINATION include/swikernel
    FILES_MATCHING PATTERN "*.h"
)

# 安装配置文件
install(DIRECTORY ${CONFIG_DIR}/
    DESTINATION etc/swikernel
    FILES_MATCHING PATTERN "*.conf"
)

# 安装脚本
install(DIRECTORY ${SCRIPT_DIR}/
    DESTINATION share/swikernel/scripts
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# 测试配置
if(BUILD_TESTING)
    enable_testing()
    
    # 查找检查工具
    find_program(VALGRIND_EXECUTABLE valgrind)
    
    # 添加测试
    add_subdirectory(${TEST_DIR})
endif()

# 包配置
include(CMakePackageConfigHelpers)

# 创建包配置文件
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/swikernel-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# 安装包配置
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/swikernel-config-version.cmake
    DESTINATION lib/cmake/swikernel
)

# 导出目标
export(EXPORT swikernel-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/swikernel-targets.cmake
)

# 安装导出
install(EXPORT swikernel-targets
    FILE swikernel-targets.cmake
    DESTINATION lib/cmake/swikernel
)

# CPack配置
set(CPACK_PACKAGE_NAME "swikernel")
set(CPACK_PACKAGE_VENDOR "SwiKernel Team")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Linux Kernel Switcher")
set(CPACK_PACKAGE_FILE_NAME "swikernel-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# 生成包
include(CPack)

# 自定义目标：格式检查
add_custom_target(format
    COMMAND find ${SOURCE_DIR} ${INCLUDE_DIR} -name '*.c' -o -name '*.h' | xargs clang-format -i
    COMMENT "Formatting source code"
)

# 自定义目标：静态分析
add_custom_target(analyze
    COMMAND find ${SOURCE_DIR} -name '*.c' -exec splint -I${INCLUDE_DIR} {} \\;
    COMMENT "Running static analysis"
)

# 自定义目标：文档生成
add_custom_target(docs
    COMMAND doxygen ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
    COMMENT "Generating documentation"
)

# 显示配置摘要
message(STATUS "")
message(STATUS "SwiKernel Configuration Summary")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Assembler: ${HAVE_NASM}")
message(STATUS "  Tests: ${BUILD_TESTING}")
message(STATUS "")