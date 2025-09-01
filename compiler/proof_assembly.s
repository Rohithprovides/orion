.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"
str_0: .string "hello\n"

.section .text
.global main
.extern printf

main:
    push %rbp
    mov %rsp, %rbp
    # Variable: a
    mov $5, %rax
    mov %rax, -8(%rbp)
    # Call out() with variable: a (type: int)
    mov -8(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Variable: b
    mov $str_0, %rax
    mov %rax, -16(%rbp)
    # Call out() with variable: b (type: string)
    mov -16(%rbp), %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Variable: c
    mov $42, %rax
    mov %rax, -24(%rbp)
    # Call out() with variable: c (type: int)
    mov -24(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    pop %rbp
    ret
