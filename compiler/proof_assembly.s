.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"
format_float: .string "%.1f\n"
dtype_int: .string "datatype: int\n"
dtype_string: .string "datatype: string\n"
dtype_bool: .string "datatype: bool\n"
dtype_float: .string "datatype: float\n"
dtype_unknown: .string "datatype: unknown\n"
float_0: .quad 4615514078110652826
float_1: .quad 4611911198408756429

.section .text
.global main
.extern printf

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Variable: a
    # Float: 3.7
    movq float_0(%rip), %rax
    mov %rax, -8(%rbp)  # store global a
    # Variable: b
    # Float: 2.1
    movq float_1(%rip), %rax
    mov %rax, -16(%rbp)  # store global b
    # Variable: c
    mov -8(%rbp), %rax  # load global a
    push %rax
    mov -16(%rbp), %rax  # load global b
    pop %rbx
    add %rbx, %rax
    mov %rax, -24(%rbp)  # store global c
    # Call out() with variable: a (type: float)
    mov -8(%rbp), %rsi
    movq -8(%rbp), %xmm0
    mov $format_float, %rdi
    mov $1, %rax
    call printf
    # Call out() with variable: b (type: float)
    mov -16(%rbp), %rsi
    movq -16(%rbp), %xmm0
    mov $format_float, %rdi
    mov $1, %rax
    call printf
    # Call out() with variable: c (type: float)
    mov -24(%rbp), %rsi
    movq -24(%rbp), %xmm0
    mov $format_float, %rdi
    mov $1, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
