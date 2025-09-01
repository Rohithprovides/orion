.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"

.section .text
.global main
.extern printf

main:
    push %rbp
    mov %rsp, %rbp
    # Variable: a
    mov %rax, -8(%rbp)
    # Call out() with variable: a (type: unknown)
    mov -8(%rbp), %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    pop %rbp
    ret
