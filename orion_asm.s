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
    # Integer binary operation
    mov $5, %rax
    push %rax
    mov $3, %rax
    pop %rbx
    cmp %rax, %rbx
    jg gt_true_0
    mov $str_false, %rax
    jmp gt_done_0
gt_true_0:
    mov $str_true, %rax
gt_done_0:
    push %rax
    # Integer binary operation
    mov $2, %rax
    push %rax
    mov $4, %rax
    pop %rbx
    cmp %rax, %rbx
    jl lt_true_1
    mov $str_false, %rax
    jmp lt_done_1
lt_true_1:
    mov $str_true, %rax
lt_done_1:
    pop %rbx
    cmp $0, %rbx
    je and_false_2
    cmp $str_false, %rbx
    je and_false_2
    cmp $0, %rax
    je and_false_2
    cmp $str_false, %rax
    je and_false_2
    mov $str_true, %rax
    jmp and_done_2
and_false_2:
    mov $str_false, %rax
and_done_2:
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
