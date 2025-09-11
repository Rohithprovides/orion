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
    # Integer binary operation
    mov $5, %rax
    push %rax
    mov $3, %rax
    pop %rbx
    cmp %rbx, %rax
    jg gt_true_0
    mov $str_false, %rax
    jmp gt_done_0
gt_true_0:
    mov $str_true, %rax
gt_done_0:
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
