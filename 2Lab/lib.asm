section .data
%define NEW_LINE `\n`
%define EXIT_CODE 60
%define MINUS '-'
%define STD_IN 0
%define STD_OUT 1
%define SYSCALL_WRITE 1
%define SYSCALL_READ 0

global exit
global string_length
global print_string
global print_char
global print_newline
global print_uint
global print_int
global string_equals
global read_char
global read_word
global parse_uint
global parse_int
global string_copy


section .text
; Принимает код возврата и завершает текущий процесс
exit:
	mov rax, EXIT_CODE
	syscall

; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
	xor rax, rax
	.loop_lenght:
    	cmp byte[rdi+rax], 0
    	jz .end
    	inc rax
    	jmp .loop_lenght
	.end:
    	ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
	push rdi
	call string_length
	pop rsi
	mov rdx, rax
	mov rax, SYSCALL_WRITE
	mov rdi, STD_OUT
	syscall
	ret


; Принимает код символа и выводит его в stdout
print_char:
	push rdi
	mov rax, SYSCALL_WRITE
	mov rsi, rsp
	mov rdi, STD_OUT
	mov rdx, 1
	syscall
	pop rdi
	ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
	mov rdi, NEW_LINE
	jmp print_char

; Выводит беззнаковое 8-байтовое число в десятичном формате
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
	push rbx
	mov rbx, 10
	xor rcx, rcx
	mov rax, rdi

	.loop_convert_print_uint:
    	xor rdx, rdx
    	div rbx

    	add dl, '0'

    	push rdx
    	inc rcx
    	test rax, rax
    	jnz .loop_convert_print_uint
	.loop_print_uint:
    	pop rdi
    	push rcx
    	call print_char
    	pop rcx

    	sub rcx, 1
    	test rcx, rcx
    	jnz .loop_print_uint
	pop rbx
	ret

; Выводит знаковое 8-байтовое число в десятичном формате
print_int:
	test rdi, rdi
	jge .print_int_uint
	push rdi
	mov rdi, MINUS
	call print_char
	pop rdi
	neg rdi
	.print_int_uint:
    	call print_uint
	ret

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
	xor rax, rax
	xor rcx, rcx
	.loop_string_equals:
     	mov al, byte[rdi]
     	cmp al, byte[rsi]
     	jnz .end_nequals
     	test al, al
     	jz .end_1equals
     	inc rdi
     	inc rsi
     	jmp .loop_string_equals
	.end_nequals:
     	xor rax, rax
     	ret
	.end_1equals:
     	inc rax
	ret

; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
	push 0
	mov rdi, STD_IN
	mov rsi, rsp
	xor rax, rax
	mov rdx, 1
	syscall
	test rax, rax
	jng .end_read_char
	mov al, byte[rsp]
	pop rdi
	ret
	.end_read_char:
    	xor rax, rax
    	pop rdi
	ret

; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор
read_word:
	push r12
	push r13
	push rbx
	push rdi
    	xor rbx, rbx
    	xor rcx, rcx
    	mov r12, rdi
    	mov r13, rsi
    	.loop_read_word:

            	push rcx
            	call read_char
            	pop rcx

            	cmp rcx, r13
            	jge .end_bad
            	cmp al, `\t`
            	jz .space
            	cmp al, `\n`
            	jz .space
            	cmp al, ' '
            	jz .space
            	test al, al
            	jz .end_good
            	mov byte[r12], al
            	inc rcx
            	inc r12
            	jmp .loop_read_word
    	.space:
            	test rcx, rcx
            	jnz .end_good
            	jmp .loop_read_word
    	.end_bad:
            	pop rax
            	xor rax, rax
            	jmp .restore_reg
    	.end_good:
            	mov  byte[r12], 0
            	pop rax
            	mov rdx, rcx
    	.restore_reg:
            	pop rbx
            	pop r13
            	pop r12
	ret


; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
	push rbx
	mov r8, 10
	xor rbx, rbx
	xor rcx, rcx
	xor rax, rax
	.loop_uint:
    	mov bl, byte[rdi + rcx]
    	sub bl, '0'
    	jb .end
    	cmp bl, 9
    	ja .end
    	mul r8
    	add rax, rbx
    	inc rcx
    	jmp .loop_uint
	.end:
    	mov rdx, rcx
    	pop rbx
	ret




; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был)
; rdx = 0 если число прочитать не удалось
parse_int:
    	xor rax, rax
    	xor rdx, rdx
    	mov al, byte[rdi]
    	cmp al, MINUS
    	jnz .positive
    	inc rdi
    	call parse_uint
    	test rax, rax
    	jz .end_bad
    	neg rax
    	inc rdx
    	.end_bad:
            	ret
    	.positive:
            	call parse_uint
            	ret

; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
	xor rcx, rcx
	.loop_copy:
    	movzx rax, byte[rdi+rcx]
    	mov byte[rsi+rcx], al
    	test rax,rax
    	jz .copy_ok
    	cmp rcx, rdx
    	jge .copy_err
    	inc rcx
    	jmp .loop_copy
	.copy_err:
    	xor rax, rax
    	ret
	.copy_ok:
    	mov rax, rcx
    	ret
