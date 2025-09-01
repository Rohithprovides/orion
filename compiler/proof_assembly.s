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
    mov $1, %rax
    mov %rax, -8(%rbp)
    # Variable: y
    mov $0, %rax
    mov %rax, -16(%rbp)
    # Call out() with variable: x (type: bool)
    mov -8(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Call out() with variable: y (type: bool)
    mov -16(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Call out(dtype(x))
    mov $dtype_bool, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out(dtype(y))
    mov $dtype_bool, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    pop %rbp
    ret
