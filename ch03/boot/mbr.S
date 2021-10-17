SECTION MBR vstart=0x7c00
global main
main:
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

; 输出绿底红色跳动字符串
; 0xb800: 'H' 0xA4 'e' 0xA4 'l' 0xA4 ...
    call display2
    jmp $

display:
    mov ax, message
    mov bp, ax
    mov cx, 14
    mov ax, 0x1301
    mov bx, 0x2
    int 0x10
    ret

display1:
    mov byte [es:0x00], 'H'
    mov byte [es:0x01], 0xA4
    mov byte [es:0x02], 'i'
    mov byte [es:0x03], 0xA4
    ret

display2:
    mov ax, 0xb800
    mov es, ax
    mov si, message
    mov di, 0
display_single:
    mov ch, 0
    mov cl, [ds:si]
    jcxz display_ok
    mov byte [es:di], cl
    mov byte [es:di+1], 0xA4
    inc si
    add di, 2
    jmp short display_single
display_ok:
    ret

message db "Hello, Tiny OS", 0
times 510-($-$$) db 0
db 0x55,0xaa
