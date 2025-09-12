.section .data
format_int: .string "%d\n"
format_str: .string "%s"
format_float: .string "%.2f\n"
dtype_int: .string "datatype: int\n"
dtype_string: .string "datatype: string\n"
dtype_bool: .string "datatype: bool\n"
dtype_float: .string "datatype: float\n"
dtype_unknown: .string "datatype: unknown\n"
str_true: .string "True\n"
str_false: .string "False\n"

.section .text
.global main
.extern printf
.extern fmod
.extern pow

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Function defined: main
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
