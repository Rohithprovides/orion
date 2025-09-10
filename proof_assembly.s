.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"
format_float: .string "%.2f\n"
dtype_int: .string "datatype: int\n"
dtype_string: .string "datatype: string\n"
dtype_bool: .string "datatype: bool\n"
dtype_float: .string "datatype: float\n"
dtype_unknown: .string "datatype: unknown\n"

.section .text
.global main
.extern printf
.extern fmod
.extern pow

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Variable: a
    mov $5, %rax
    mov %rax, -8(%rbp)  # store global a
    # Call out() with variable: a (type: int)
    mov -8(%rbp), %rsi
    mov $format_int, %rdi
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
