.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"
dtype_int: .string "datatype: int\n"
dtype_string: .string "datatype: string\n"
dtype_bool: .string "datatype: bool\n"
dtype_float: .string "datatype: float\n"
dtype_unknown: .string "datatype: unknown\n"

.section .text
.global main
.extern printf

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Variable: a
    mov $15, %rax
    mov %rax, -8(%rbp)  # store global a
    # Variable: b
    mov $3, %rax
    mov %rax, -16(%rbp)  # store global b
    # Variable: c
    mov -8(%rbp), %rax  # load global a
    push %rax
    mov -16(%rbp), %rax  # load global b
    pop %rbx
    add %rbx, %rax
    mov %rax, -24(%rbp)  # store global c
    # Call out() with variable: c (type: int)
    mov -24(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
