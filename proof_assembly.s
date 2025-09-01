.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"
str_0: .string "hello\n"
str_1: .string "world\n"

.section .text
.global main
.extern printf

main:
    push %rbp
    mov %rsp, %rbp
    # Variable: a
    mov $str_0, %rax
    mov %rax, -8(%rbp)
    # Call out() with variable: a (type: string)
    mov -8(%rbp), %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Variable: b
    mov $str_1, %rax
    mov %rax, -16(%rbp)
    # Call out() with variable: b (type: string)
    mov -16(%rbp), %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    pop %rbp
    ret
