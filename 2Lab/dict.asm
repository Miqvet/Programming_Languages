%include "lib.inc"


section .text

global find_word

find_word:
    push r12
    push r13
    mov r12, rdi
    mov r13, rsi
    .loop:
        test r13, r13
        jz .end_err

        mov rdi, r12
        lea rsi, [r13 + 8]
        call string_equals
        test rax, rax
        jnz .end_good
   

        mov r13, [r13]
        jmp .loop
    .end_err:
        xor rax,rax
        jmp .restore_reg
    .end_good:
        mov rax, r13
    .restore_reg:
        pop r13
        pop r12
        ret
