.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"
str_0: .string "5\n"

.section .text
.global main
.extern printf

main:
    push %rbp
    mov %rsp, %rbp
    # Variable: a
    mov $str_0, %rax
    mov %rax, -8(%rbp)
    # Call out() with variable: a
    mov -8(%rbp), %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    pop %rbp
    ret
