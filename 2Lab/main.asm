%include "lib.inc"
%include "dict.inc"
%include "words.inc"

section .bss
buffer resb 256

section .rodata
read_err: dq `Error of reading\n`, 0
find_err: dq `No such  word\n`, 0

section .text

global _start

_start:
	mov rdi, buffer
	mov rsi, 256
	call read_word
	test rax, rax
	jz .err_read

	mov rdi, rax
	mov rsi, first_word
	call find_word
	test rax, rax
	jz .err_search

	mov rdi, rax
	add rdi, 8
	push rdi
	call string_length
	pop rdi
	add rdi, rax
	inc rdi
	call print_string
	call print_newline
	call exit

   .err_read:
    	mov rdi, read_err
    	jmp .final

   .err_search:
    	mov rdi, find_err

   .final:
    	call print_error
    	call exit


print_error:
	push rdi
	call string_length
	pop rsi
	mov rdx, rax
	mov rax, 1
	mov rdi, 2
	syscall
	ret
