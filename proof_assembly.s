.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"
format_float: .string "%.1f\n"
dtype_int: .string "datatype: int\n"
dtype_string: .string "datatype: string\n"
dtype_bool: .string "datatype: bool\n"
dtype_float: .string "datatype: float\n"
dtype_unknown: .string "datatype: unknown\n"
float_0: .quad 4612811918334230528
float_1: .quad 4608083138725491507

.section .text
.global main
.extern printf

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Function defined: main
    # Explicit call to main()
    # Executing function call: main
    # Variable: a
    # Float: 2.5
    movq float_0(%rip), %rax
    mov %rax, -8(%rbp)  # store local a
    # Variable: b
    # Float: 1.2
    movq float_1(%rip), %rax
    mov %rax, -16(%rbp)  # store local b
    # Floating-point binary operation
    mov -8(%rbp), %rax  # load local a
    movq %rax, %xmm0  # Load float left operand
    subq $8, %rsp
    movsd %xmm0, (%rsp)  # Save left operand on stack
    mov -16(%rbp), %rax  # load local b
    movq %rax, %xmm1  # Load float right operand
    movsd (%rsp), %xmm0  # Restore left operand
    addq $8, %rsp
    addsd %xmm1, %xmm0  # Float addition
    movq %xmm0, %rax  # Store float result
    # Call out() with expression result
    movq %rax, %xmm0  # Load float result into XMM register
    mov $format_float, %rdi
    mov $1, %rax  # Number of vector registers used
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
