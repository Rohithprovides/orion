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
    # Integer binary operation
    mov $5, %rax
    push %rax
    mov $5, %rax
    pop %rbx
    # Unsupported binary operation
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Integer binary operation
    mov $5, %rax
    push %rax
    mov $3, %rax
    pop %rbx
    # Unsupported binary operation
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Integer binary operation
    mov $10, %rax
    push %rax
    mov $5, %rax
    pop %rbx
    # Unsupported binary operation
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Integer binary operation
    mov $5, %rax
    push %rax
    mov $10, %rax
    pop %rbx
    # Unsupported binary operation
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Integer binary operation
    # Integer binary operation
    mov $3, %rax
    push %rax
    mov $2, %rax
    pop %rbx
    add %rbx, %rax
    push %rax
    # Integer binary operation
    mov $10, %rax
    push %rax
    mov $2, %rax
    pop %rbx
    mov %rax, %rcx
    mov %rbx, %rax
    xor %rdx, %rdx
    idiv %rcx
    pop %rbx
    # Unsupported binary operation
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
