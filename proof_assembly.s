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
    mov $5, %rax
    mov %rax, -8(%rbp)
    # Variable: b
    mov $6, %rax
    mov %rax, -16(%rbp)
    # Tuple assignment
    mov -16(%rbp), %rax
    mov %rax, -24(%rbp)  # temp 0
    mov -8(%rbp), %rax
    mov %rax, -32(%rbp)  # temp 1
    mov -24(%rbp), %rax  # load temp 0
    mov %rax, -8(%rbp)  # assign to a
    mov -32(%rbp), %rax  # load temp 1
    mov %rax, -16(%rbp)  # assign to b
    # Call out() with variable: a (type: int)
    mov -8(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    pop %rbp
    ret
