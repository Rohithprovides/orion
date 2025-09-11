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
    mov $str_true, %rax
    push %rax
    mov $str_false, %rax
    pop %rbx
    cmp $0, %rbx
    je and_false_0
    cmp $str_false, %rbx
    je and_false_0
    cmp $0, %rax
    je and_false_0
    cmp $str_false, %rax
    je and_false_0
    mov $str_true, %rax
    jmp and_done_0
and_false_0:
    mov $str_false, %rax
and_done_0:
    cmp $0, %rax
    je not_true_1
    cmp $str_false, %rax
    je not_true_1
    mov $str_false, %rax
    jmp not_done_1
not_true_1:
    mov $str_true, %rax
not_done_1:
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
