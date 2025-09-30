; src/asm_optimized/memory_ops.asm
section .text
    global fast_memset
    global fast_memcmp
    global fast_memmove
    global allocate_aligned_memory

; 快速内存设置
; void fast_memset(void *ptr, int value, size_t n)
fast_memset:
    push rbp
    mov rbp, rsp
    push rdi
    push rcx
    
    mov rdi, [rbp + 16]  ; ptr
    mov al, [rbp + 24]   ; value (低8位)
    mov rcx, [rbp + 32]  ; n
    
    ; 扩展值到整个rax
    mov ah, al
    mov bx, ax
    shl eax, 16
    mov ax, bx
    
    ; 对齐检查
    test rdi, 7
    jnz .byte_set
    
    ; 对齐设置 - 8字节一次
    mov rcx, [rbp + 32]
    shr rcx, 3           ; 除以8
    jz .byte_set
    
.quad_set:
    mov [rdi], rax
    add rdi, 8
    dec rcx
    jnz .quad_set
    
    ; 处理剩余字节
    mov rcx, [rbp + 32]
    and rcx, 7
    jz .done
    
.byte_set:
    mov [rdi], al
    inc rdi
    dec rcx
    jnz .byte_set
    
.done:
    pop rcx
    pop rdi
    pop rbp
    ret

; 快速内存比较
; int fast_memcmp(const void *s1, const void *s2, size_t n)
fast_memcmp:
    push rbp
    mov rbp, rsp
    push rdi
    push rsi
    push rcx
    
    mov rdi, [rbp + 16]  ; s1
    mov rsi, [rbp + 24]  ; s2
    mov rcx, [rbp + 32]  ; n
    
    ; 对齐检查
    test rdi, 7
    jnz .byte_compare
    test rsi, 7
    jnz .byte_compare
    
    ; 对齐比较 - 8字节一次
    mov rcx, [rbp + 32]
    shr rcx, 3
    jz .byte_compare
    
.quad_compare:
    mov rax, [rdi]
    mov rbx, [rsi]
    cmp rax, rbx
    jne .difference
    add rdi, 8
    add rsi, 8
    dec rcx
    jnz .quad_compare
    
    ; 处理剩余字节
    mov rcx, [rbp + 32]
    and rcx, 7
    jz .equal
    
.byte_compare:
    mov al, [rdi]
    mov bl, [rsi]
    cmp al, bl
    jne .difference
    inc rdi
    inc rsi
    dec rcx
    jnz .byte_compare
    
.equal:
    xor rax, rax
    jmp .done
    
.difference:
    movzx rax, al
    movzx rbx, bl
    sub rax, rbx
    
.done:
    pop rcx
    pop rsi
    pop rdi
    pop rbp
    ret

; 快速内存移动（处理重叠）
; void fast_memmove(void *dest, const void *src, size_t n)
fast_memmove:
    push rbp
    mov rbp, rsp
    push rdi
    push rsi
    push rcx
    
    mov rdi, [rbp + 16]  ; dest
    mov rsi, [rbp + 24]  ; src
    mov rcx, [rbp + 32]  ; n
    
    ; 检查重叠
    cmp rdi, rsi
    jb .forward_copy     ; dest < src，向前复制
    
    ; 向后复制（处理重叠）
    lea rdi, [rdi + rcx - 1]
    lea rsi, [rsi + rcx - 1]
    std                  ; 设置方向标志（向后）
    
.backward_copy:
    mov al, [rsi]
    mov [rdi], al
    dec rsi
    dec rdi
    dec rcx
    jnz .backward_copy
    jmp .done
    
.forward_copy:
    cld                  ; 清除方向标志（向前）
    rep movsb
    
.done:
    pop rcx
    pop rsi
    pop rdi
    pop rbp
    ret

; 分配对齐内存
; void* allocate_aligned_memory(size_t size, size_t alignment)
allocate_aligned_memory:
    push rbp
    mov rbp, rsp
    
    mov rdi, [rbp + 16]  ; size
    mov rsi, [rbp + 24]  ; alignment
    
    ; 使用posix_memalign系统调用
    mov rax, 12          ; brk系统调用（简化实现）
    syscall
    
    pop rbp
    ret