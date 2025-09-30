; src/system/syscalls.asm
section .text
    global fast_syscall
    global fast_file_check
    global fast_get_time
    global syscall_execute
    global get_system_info

; 高性能系统调用封装
; 参数：rax=系统调用号, rdi, rsi, rdx, r10, r8, r9=参数
fast_syscall:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    
    ; 执行系统调用:cite[1]:cite[2]
    syscall
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; 快速文件存在检查
; 参数：rdi=文件路径
; 返回：rax=0存在, -1不存在
fast_file_check:
    mov rax, 2          ; sys_open
    mov rsi, 0          ; O_RDONLY
    mov rdx, 0          ; 模式
    syscall
    
    cmp rax, 0
    jl .not_exists
    
    ; 文件存在，关闭文件
    mov rdi, rax
    mov rax, 3          ; sys_close
    syscall
    mov rax, 0
    ret
    
.not_exists:
    mov rax, -1
    ret

; 快速获取时间字符串
; 参数：rdi=缓冲区地址
; 格式：YYYY-MM-DD HH:MM:SS
fast_get_time:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    
    ; 获取时间结构体
    mov rax, 96         ; sys_gettimeofday
    lea rdi, [rbp-16]   ; timeval 结构体
    xor rsi, rsi        ; 时区
    syscall
    
    ; 转换为可读格式
    mov rdi, [rbp-16]   ; 时间戳
    mov rsi, [rsp+16]   ; 缓冲区地址
    call format_time
    
    add rsp, 32
    pop rbp
    ret

; 执行系统调用并处理错误
; 参数：rdi=系统调用号, rsi=参数结构体
syscall_execute:
    push rbp
    mov rbp, rsp
    
    mov rax, rdi        ; 系统调用号
    mov rdi, rsi        ; 第一个参数
    
    ; 根据参数数量执行调用
    cmp rdx, 1
    je .one_arg
    cmp rdx, 2
    je .two_args
    cmp rdx, 3
    je .three_args
    
.one_arg:
    syscall
    jmp .done
    
.two_args:
    mov rsi, [rsp+24]   ; 第二个参数
    syscall
    jmp .done
    
.three_args:
    mov rsi, [rsp+24]   ; 第二个参数
    mov rdx, [rsp+32]   ; 第三个参数
    syscall
    
.done:
    ; 检查错误
    cmp rax, 0
    jl .error
    pop rbp
    ret
    
.error:
    neg rax
    mov [errno], eax
    mov rax, -1
    pop rbp
    ret

; 获取系统信息
; 参数：rdi=信息类型, rsi=输出缓冲区
get_system_info:
    push rbp
    mov rbp, rsp
    
    mov rax, 99         ; sys_sysinfo
    mov rdi, rsi        ; sysinfo 结构体
    syscall
    
    pop rbp
    ret

; 内部函数：格式化时间
format_time:
    push rbp
    mov rbp, rsp
    
    ; 使用C库函数格式化时间（实际项目中应使用系统调用）
    ; 这里简化为返回固定格式
    mov byte [rsi], '2'
    mov byte [rsi+1], '0'
    mov byte [rsi+2], '2'
    mov byte [rsi+3], '4'
    mov byte [rsi+4], '-'
    mov byte [rsi+5], '0'
    mov byte [rsi+6], '1'
    mov byte [rsi+7], '-'
    mov byte [rsi+8], '0'
    mov byte [rsi+9], '1'
    mov byte [rsi+10], ' '
    mov byte [rsi+11], '0'
    mov byte [rsi+12], '0'
    mov byte [rsi+13], ':'
    mov byte [rsi+14], '0'
    mov byte [rsi+15], '0'
    mov byte [rsi+16], ':'
    mov byte [rsi+17], '0'
    mov byte [rsi+18], '0'
    mov byte [rsi+19], 0
    
    pop rbp
    ret