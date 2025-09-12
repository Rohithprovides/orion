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
str_0: .string "x after += 5: works!\n"
str_1: .string "y after *= 2: works!\n"
str_2: .string "z after /= 3: works!\n"

.section .text
.global main
.extern printf
.extern fmod
.extern pow

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Function defined: test
    # User-defined function call: test
    # Executing function call: test
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
    # Call out() with string
    mov $str_0, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Variable: y
    mov $20, %rax
    mov %rax, -24(%rbp)  # store local y
    # Variable: y
    # Integer binary operation
    mov -24(%rbp), %rax  # load local y
    push %rax
    mov $2, %rax
    pop %rbx
    imul %rbx, %rax
    mov %rax, -32(%rbp)  # store local y
    # Call out() with string
    mov $str_1, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Variable: z
    mov $15, %rax
    mov %rax, -40(%rbp)  # store local z
    # Variable: z
    # Integer binary operation
    mov -40(%rbp), %rax  # load local z
    push %rax
    mov $3, %rax
    pop %rbx
    mov %rax, %rcx
    mov %rbx, %rax
    xor %rdx, %rdx
    idiv %rcx
    mov %rax, -48(%rbp)  # store local z
    # Call out() with string
    mov $str_2, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
