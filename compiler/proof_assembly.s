.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"
dtype_int: .string "datatype: int\n"
dtype_string: .string "datatype: string\n"
dtype_bool: .string "datatype: bool\n"
dtype_float: .string "datatype: float\n"
dtype_unknown: .string "datatype: unknown\n"
str_0: .string "hello world\n"
str_1: .string "Testing dtype() function:\n"

.section .text
.global main
.extern printf

main:
    push %rbp
    mov %rsp, %rbp
    # Variable: x
    mov $42, %rax
    mov %rax, -8(%rbp)
    # Variable: y
    mov $str_0, %rax
    mov %rax, -16(%rbp)
    # Variable: z
    mov $0, %rax
    mov %rax, -24(%rbp)
    # Variable: w
    mov $1, %rax
    mov %rax, -32(%rbp)
    # Call out() with string
    mov $str_1, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out(dtype(x))
    mov $dtype_int, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out(dtype(y))
    mov $dtype_string, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out(dtype(z))
    mov $dtype_bool, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out(dtype(w))
    mov $dtype_bool, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    pop %rbp
    ret
