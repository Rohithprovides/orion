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
    sub $64, %rsp
    # Function defined: main
    # Function defined: hello
    # Explicit call to main()
    # Executing function call: main
    # Variable: a
    mov $5, %rax
    mov %rax, -8(%rbp)
    # Call out() with variable: a (type: int)
    mov -8(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Function 'hello' defined but not executed
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
