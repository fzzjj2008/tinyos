SECTION MBR vstart=0x7c00
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00

; 屏幕初始化，清屏
    mov ax, 0x600
    mov bx, 0x700
    mov cx, 0
    mov dx, 0x184f
    int 0x10

; 设置光标位置，BH = 页号，DH = 行，DL = 列
    mov ah, 2
    mov bh, 0
    mov dx, 0x0a20
    int 0x10

; 显示字符串，ES:BP = 串地址，CX = 串长度，BL = 字符属性（黑底绿字）
    mov ax, message
    mov bp, ax
    mov cx, 14
    mov ax, 0x1301
    mov bx, 0x2
    int 0x10

    jmp $

    message db "Hello, Tiny OS"
    times 510-($-$$) db 0
    db 0x55, 0xaa
