; src/asm_optimized/string_ops.asm
section .text
    global asm_string_match
    global fast_memcpy
    global fast_strlen
    global fast_strcmp
    global fast_strstr

; 快速字符串前缀匹配:cite[1]
; int asm_string_match(const char *str, const char *prefix)
asm_string_match:
    push rbp
    mov rbp, rsp
    push rdi
    push rsi
    push rbx
    
    mov rdi, [rbp + 16]  ; str
    mov rsi, [rbp + 24]  ; prefix
    
.test_loop:
    mov al, [rsi]        ; 加载前缀字符
    test al, al          ; 检查是否到达前缀结尾
    jz .match_success
    
    mov bl, [rdi]        ; 加载字符串字符
    test bl, bl          ; 检查是否到达字符串结尾
    jz .match_fail
    
    cmp al, bl           ; 比较字符
    jne .match_fail      ; 不匹配
    
    inc rdi              ; 下一个字符
    inc rsi
    jmp .test_loop
    
.match_success:
    mov rax, 1
    jmp .done
    
.match_fail:
    mov rax, 0
    
.done:
    pop rbx
    pop rsi
    pop rdi
    pop rbp
    ret

; 快速内存复制
; void fast_memcpy(void *dest, const void *src, size_t n)
fast_memcpy:
    push rbp
    mov rbp, rsp
    push rdi
    push rsi
    push rcx
    
    mov rdi, [rbp + 16]  ; dest
    mov rsi, [rbp + 24]  ; src  
    mov rcx, [rbp + 32]  ; n
    
    ; 检查对齐
    test rdi, 7
    jnz .byte_copy
    test rsi, 7
    jnz .byte_copy
    
    ; 对齐复制 - 8字节一次
    mov rcx, [rbp + 32]
    shr rcx, 3           ; 除以8
    jz .byte_copy
    
.quad_copy:
    mov rax, [rsi]
    mov [rdi], rax
    add rsi, 8
    add rdi, 8
    dec rcx
    jnz .quad_copy
    
    ; 处理剩余字节
    mov rcx, [rbp + 32]
    and rcx, 7
    jz .done
    
.byte_copy:
    mov al, [rsi]
    mov [rdi], al
    inc rsi
    inc rdi
    dec rcx
    jnz .byte_copy
    
.done:
    pop rcx
    pop rsi
    pop rdi
    pop rbp
    ret

; 快速字符串长度计算
; size_t fast_strlen(const char *str)
fast_strlen:
    push rbp
    mov rbp, rsp
    push rdi
    
    mov rdi, [rbp + 16]  ; str
    xor rax, rax         ; 计数器清零
    
.test_char:
    cmp byte [rdi + rax], 0
    je .done
    inc rax
    jmp .test_char
    
.done:
    pop rdi
    pop rbp
    ret

; 快速字符串比较
; int fast_strcmp(const char *s1, const char *s2)
fast_strcmp:
    push rbp
    mov rbp, rsp
    push rdi
    push rsi
    
    mov rdi, [rbp + 16]  ; s1
    mov rsi, [rbp + 24]  ; s2
    
.compare_loop:
    mov al, [rdi]
    mov bl, [rsi]
    cmp al, bl
    jne .not_equal
    test al, al
    jz .equal
    inc rdi
    inc rsi
    jmp .compare_loop

.equal:
    xor rax, rax
    jmp .done

.not_equal:
    movzx rax, al
    movzx rbx, bl
    sub rax, rbx

.done:
    pop rsi
    pop rdi
    pop rbp
    ret

; 快速子字符串查找
; char* fast_strstr(const char *haystack, const char *needle)
fast_strstr:
    push rbp
    mov rbp, rsp
    push rdi
    push rsi
    push rbx
    
    mov rdi, [rbp + 16]  ; haystack
    mov rsi, [rbp + 24]  ; needle
    
    ; 检查空指针
    test rdi, rdi
    jz .not_found
    test rsi, rsi
    jz .not_found
    
    ; 获取 needle 长度
    mov rdx, rsi
    call fast_strlen
    mov rcx, rax         ; needle 长度
    test rcx, rcx
    jz .empty_needle
    
    ; 主搜索循环
.search_loop:
    mov al, [rdi]
    test al, al
    jz .not_found
    
    ; 比较当前位置
    mov rbx, rdi
    mov rdx, rsi
    mov r8, rcx
    
.compare:
    mov al, [rbx]
    mov ah, [rdx]
    cmp al, ah
    jne .next_char
    inc rbx
    inc rdx
    dec r8
    jnz .compare
    
    ; 找到匹配
    mov rax, rdi
    jmp .done
    
.next_char:
    inc rdi
    jmp .search_loop
    
.empty_needle:
    mov rax, rdi
    jmp .done
    
.not_found:
    xor rax, rax
    
.done:
    pop rbx
    pop rsi
    pop rdi
    pop rbp
    ret