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
    mov $999, %rax
    mov %rax, -8(%rbp)  # assign to first
    mov %rax, -16(%rbp)  # assign to second
    mov $0, %rax
    pop %rbp
    ret
