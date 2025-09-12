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
str_0: .string "Final value: works!\n"

.section .text
.global main
.extern printf
.extern fmod
.extern pow

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Function defined: example
    # User-defined function call: example
    # Executing function call: example
    # Variable: x
    mov $10, %rax
    mov %rax, -8(%rbp)  # store local x
    # Variable: x
    # Integer binary operation
    mov -8(%rbp), %rax  # load local x
    push %rax
    mov $5, %rax
    pop %rbx
    add %rbx, %rax
    mov %rax, -16(%rbp)  # store local x
    # Variable: x
    # Integer binary operation
    mov -16(%rbp), %rax  # load local x
    push %rax
    mov $2, %rax
    pop %rbx
    imul %rbx, %rax
    mov %rax, -24(%rbp)  # store local x
    # Variable: x
    # Integer binary operation
    mov -24(%rbp), %rax  # load local x
    push %rax
    mov $6, %rax
    pop %rbx
    mov %rax, %rcx
    mov %rbx, %rax
    xor %rdx, %rdx
    idiv %rcx
    mov %rax, -32(%rbp)  # store local x
    # Call out() with string
    mov $str_0, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
