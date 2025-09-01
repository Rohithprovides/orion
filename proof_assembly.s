.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"

.section .text
.global main
.extern printf

main:
    push %rbp
    mov %rsp, %rbp
    # Chain assignment
    mov $5, %rax
    mov %rax, -8(%rbp)  # assign to a
    mov %rax, -16(%rbp)  # assign to b
    # Call out() with variable: a (type: int)
    mov -8(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    pop %rbp
    ret
