.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"
format_float: .string "%.2f\n"
dtype_int: .string "datatype: int\n"
dtype_string: .string "datatype: string\n"
dtype_bool: .string "datatype: bool\n"
dtype_float: .string "datatype: float\n"
dtype_unknown: .string "datatype: unknown\n"
float_0: .quad 4612811918334230528

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
    # Explicit call to main()
    # Executing function call: main
    # Variable: a
    # Float: 2.5
    movq float_0(%rip), %rax
    mov %rax, -8(%rbp)  # store local a
    # Variable: b
    mov $2, %rax
    mov %rax, -16(%rbp)  # store local b
    # Floating-point binary operation
    mov -8(%rbp), %rax  # load local a
    movq %rax, %xmm0  # Load float left operand
    subq $8, %rsp
    movsd %xmm0, (%rsp)  # Save left operand on stack
    mov -16(%rbp), %rax  # load local b
    cvtsi2sd %rax, %xmm1  # Convert int to float (right)
    movsd (%rsp), %xmm0  # Restore left operand
    addq $8, %rsp
    addsd %xmm1, %xmm0  # Float addition
    movq %xmm0, %rax  # Store float result
    # Call out() with expression result
    movq %rax, %xmm0  # Load float result into XMM register
    mov $format_float, %rdi
    mov $1, %rax  # Number of vector registers used
    call printf
    # Floating-point binary operation
    mov -8(%rbp), %rax  # load local a
    movq %rax, %xmm0  # Load float left operand
    subq $8, %rsp
    movsd %xmm0, (%rsp)  # Save left operand on stack
    mov -16(%rbp), %rax  # load local b
    cvtsi2sd %rax, %xmm1  # Convert int to float (right)
    movsd (%rsp), %xmm0  # Restore left operand
    addq $8, %rsp
    subsd %xmm1, %xmm0  # Float subtraction
    movq %xmm0, %rax  # Store float result
    # Call out() with expression result
    movq %rax, %xmm0  # Load float result into XMM register
    mov $format_float, %rdi
    mov $1, %rax  # Number of vector registers used
    call printf
    # Floating-point binary operation
    mov -8(%rbp), %rax  # load local a
    movq %rax, %xmm0  # Load float left operand
    subq $8, %rsp
    movsd %xmm0, (%rsp)  # Save left operand on stack
    mov -16(%rbp), %rax  # load local b
    cvtsi2sd %rax, %xmm1  # Convert int to float (right)
    movsd (%rsp), %xmm0  # Restore left operand
    addq $8, %rsp
    mulsd %xmm1, %xmm0  # Float multiplication
    movq %xmm0, %rax  # Store float result
    # Call out() with expression result
    movq %rax, %xmm0  # Load float result into XMM register
    mov $format_float, %rdi
    mov $1, %rax  # Number of vector registers used
    call printf
    # Floating-point binary operation
    mov -8(%rbp), %rax  # load local a
    movq %rax, %xmm0  # Load float left operand
    subq $8, %rsp
    movsd %xmm0, (%rsp)  # Save left operand on stack
    mov -16(%rbp), %rax  # load local b
    cvtsi2sd %rax, %xmm1  # Convert int to float (right)
    movsd (%rsp), %xmm0  # Restore left operand
    addq $8, %rsp
    divsd %xmm1, %xmm0  # Float division
    movq %xmm0, %rax  # Store float result
    # Call out() with expression result
    movq %rax, %xmm0  # Load float result into XMM register
    mov $format_float, %rdi
    mov $1, %rax  # Number of vector registers used
    call printf
    # Floating-point binary operation
    mov -16(%rbp), %rax  # load local b
    cvtsi2sd %rax, %xmm0  # Convert int to float (left)
    subq $8, %rsp
    movsd %xmm0, (%rsp)  # Save left operand on stack
    mov -8(%rbp), %rax  # load local a
    movq %rax, %xmm1  # Load float right operand
    movsd (%rsp), %xmm0  # Restore left operand
    addq $8, %rsp
    divsd %xmm1, %xmm0  # Float division
    movq %xmm0, %rax  # Store float result
    # Call out() with expression result
    movq %rax, %xmm0  # Load float result into XMM register
    mov $format_float, %rdi
    mov $1, %rax  # Number of vector registers used
    call printf
    # Floating-point binary operation
    mov -8(%rbp), %rax  # load local a
    movq %rax, %xmm0  # Load float left operand
    subq $8, %rsp
    movsd %xmm0, (%rsp)  # Save left operand on stack
    mov -16(%rbp), %rax  # load local b
    cvtsi2sd %rax, %xmm1  # Convert int to float (right)
    movsd (%rsp), %xmm0  # Restore left operand
    addq $8, %rsp
    # Float power - save registers and call pow
    subq $16, %rsp  # Align stack
    movsd %xmm0, (%rsp)  # Save base
    movsd %xmm1, 8(%rsp)  # Save exponent
    movsd (%rsp), %xmm0  # Load base for pow
    movsd 8(%rsp), %xmm1  # Load exponent for pow
    call pow  # Call C library pow function
    addq $16, %rsp  # Restore stack
    movq %xmm0, %rax  # Store float result
    # Call out() with expression result
    movq %rax, %xmm0  # Load float result into XMM register
    mov $format_float, %rdi
    mov $1, %rax  # Number of vector registers used
    call printf
    # Floating-point binary operation
    mov -8(%rbp), %rax  # load local a
    movq %rax, %xmm0  # Load float left operand
    subq $8, %rsp
    movsd %xmm0, (%rsp)  # Save left operand on stack
    mov -16(%rbp), %rax  # load local b
    cvtsi2sd %rax, %xmm1  # Convert int to float (right)
    movsd (%rsp), %xmm0  # Restore left operand
    addq $8, %rsp
    # Float modulo - save registers and call fmod
    subq $16, %rsp  # Align stack
    movsd %xmm0, (%rsp)  # Save first operand
    movsd %xmm1, 8(%rsp)  # Save second operand
    movsd (%rsp), %xmm0  # Load first arg for fmod
    movsd 8(%rsp), %xmm1  # Load second arg for fmod
    call fmod  # Call C library fmod function
    addq $16, %rsp  # Restore stack
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
