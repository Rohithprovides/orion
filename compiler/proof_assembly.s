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
    mov $10, %rax
    mov %rax, -8(%rbp)
    # Variable: b
    mov $20, %rax
    mov %rax, -16(%rbp)
    # Call out() with variable: a (type: int)
    mov -8(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Call out() with variable: b (type: int)
    mov -16(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    pop %rbp
    ret
