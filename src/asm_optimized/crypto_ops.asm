; src/asm_optimized/crypto_ops.asm
section .text
    global simple_hash
    global xor_encrypt
    global calculate_crc32

; 简单哈希函数（用于内部使用，非加密安全）
; unsigned int simple_hash(const char *data, size_t len)
simple_hash:
    push rbp
    mov rbp, rsp
    push rdi
    push rsi
    push rcx
    push rbx
    
    mov rdi, [rbp + 16]  ; data
    mov rcx, [rbp + 24]  ; len
    xor rax, rax         ; 哈希值
    xor rbx, rbx         ; 临时值
    
.hash_loop:
    mov bl, [rdi]
    add rax, rbx
    rol rax, 13          ; 左旋转13位
    xor rax, rbx
    inc rdi
    loop .hash_loop
    
    pop rbx
    pop rcx
    pop rsi
    pop rdi
    pop rbp
    ret

; XOR加密/解密
; void xor_encrypt(char *data, size_t len, const char *key, size_t key_len)
xor_encrypt:
    push rbp
    mov rbp, rsp
    push rdi
    push rsi
    push rcx
    push rdx
    push r8
    
    mov rdi, [rbp + 16]  ; data
    mov rcx, [rbp + 24]  ; len
    mov rdx, [rbp + 32]  ; key
    mov r8, [rbp + 40]   ; key_len
    
    test rcx, rcx
    jz .done
    test r8, r8
    jz .done
    
    xor r9, r9           ; 密钥索引
    
.encrypt_loop:
    mov al, [rdi]        ; 数据字节
    mov bl, [rdx + r9]   ; 密钥字节
    xor al, bl
    mov [rdi], al
    
    inc rdi
    inc r9
    cmp r9, r8
    jb .next_key
    xor r9, r9           ; 重置密钥索引
    
.next_key:
    dec rcx
    jnz .encrypt_loop
    
.done:
    pop r8
    pop rdx
    pop rcx
    pop rsi
    pop rdi
    pop rbp
    ret

; CRC32计算
; unsigned int calculate_crc32(const unsigned char *data, size_t len)
calculate_crc32:
    push rbp
    mov rbp, rsp
    push rdi
    push rsi
    push rcx
    push rbx
    
    mov rdi, [rbp + 16]  ; data
    mov rcx, [rbp + 24]  ; len
    mov rax, 0xFFFFFFFF  ; CRC初始值
    
    ; CRC32查找表（简化版，实际应使用完整表）
    lea rbx, [crc_table]
    
.crc_loop:
    mov dl, [rdi]
    xor dl, al           ; dl = data ^ crc
    
    movzx rdx, dl
    mov edx, [rbx + rdx * 4]
    
    shr eax, 8
    xor eax, edx
    
    inc rdi
    dec rcx
    jnz .crc_loop
    
    not eax              ; 最终取反
    
    pop rbx
    pop rcx
    pop rsi
    pop rdi
    pop rbp
    ret

section .data
crc_table:
    dd 0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA
    ; 简化的CRC表，实际应包含256个条目