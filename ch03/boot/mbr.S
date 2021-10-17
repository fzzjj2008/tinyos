SECTION MBR vstart=0x7c00
    mov ax, cs
    mov ds, ax
    mov ss, ax
    mov sp, 0x7c00
    mov ax, 0xb800
    mov es, ax

; 屏幕初始化，清屏
    mov ax, 0x600
    mov bx, 0x700
    mov cx, 0
    mov dx, 0x184f
    int 0x10

; 输出绿底红色跳动字符串
; 0xb800: 'H' 0xA4 'e' 0xA4 'l' 0xA4 ...
    mov byte [es:0x00], 'H'
    mov byte [es:0x01], 0xA4
    mov byte [es:0x02], 'i'
    mov byte [es:0x03], 0xA4

    jmp $

message db "Hello, Tiny OS"
times 510-($-$$) db 0
db 0x55,0xaa
