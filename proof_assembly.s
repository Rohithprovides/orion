.section .data
format_int: .string "%d\n"
format_str: .string "%s\n"
format_float: .string "%.1f\n"
dtype_int: .string "datatype: int\n"
dtype_string: .string "datatype: string\n"
dtype_bool: .string "datatype: bool\n"
dtype_float: .string "datatype: float\n"
dtype_unknown: .string "datatype: unknown\n"
float_0: .quad 4612586738352862003
float_1: .quad 4609884578576439706
float_2: .quad 4617878467915022336
float_3: .quad 4611686018427387904
float_4: .quad 4621819117588971520
float_5: .quad 4613937818241073152

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
    # Floating-point binary operation
    # Float: 2.4
    movq float_0(%rip), %rax
    movq %rax, %xmm0  # Load float left operand
    subq $8, %rsp
    movsd %xmm0, (%rsp)  # Save left operand on stack
    # Float: 1.6
    movq float_1(%rip), %rax
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
    # Floating-point binary operation
    # Float: 5.5
    movq float_2(%rip), %rax
    movq %rax, %xmm0  # Load float left operand
    subq $8, %rsp
    movsd %xmm0, (%rsp)  # Save left operand on stack
    # Float: 2
    movq float_3(%rip), %rax
    movq %rax, %xmm1  # Load float right operand
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
    # Float: 10
    movq float_4(%rip), %rax
    movq %rax, %xmm0  # Load float left operand
    subq $8, %rsp
    movsd %xmm0, (%rsp)  # Save left operand on stack
    # Float: 3
    movq float_5(%rip), %rax
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
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
