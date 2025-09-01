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
    # Variable: x
    mov $10, %rax
    mov %rax, -8(%rbp)
    # Variable: y
    mov -8(%rbp), %rax
    mov %rax, -16(%rbp)
    # Variable: z
    mov -16(%rbp), %rax
    mov %rax, -24(%rbp)
    # Call out() with variable: z (type: int)
    mov -24(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    pop %rbp
    ret
