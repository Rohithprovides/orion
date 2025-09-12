.section .data
format_int: .string "%d\n"
format_str: .string "%s"
format_float: .string "%.2f\n"
dtype_int: .string "datatype: int\n"
dtype_string: .string "datatype: string\n"
dtype_bool: .string "datatype: bool\n"
dtype_float: .string "datatype: float\n"
dtype_list: .string "datatype: list\n"
dtype_unknown: .string "datatype: unknown\n"
str_true: .string "True\n"
str_false: .string "False\n"
str_index_error: .string "Index Error\n"

.section .text
.global main
.extern printf
.extern malloc
.extern exit
.extern fmod
.extern pow

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Variable: a
    # List literal with 3 elements
    mov $32, %rdi  # Allocation size
    call malloc  # Allocate memory for list
    mov %rax, %rbx  # Save list pointer
    movq $3, (%rbx)  # Store list size
    # Store element 0
    push %rbx  # Save list pointer
    mov $1, %rax
    pop %rbx  # Restore list pointer
    movq %rax, 8(%rbx)  # Store element 0
    # Store element 1
    push %rbx  # Save list pointer
    mov $2, %rax
    pop %rbx  # Restore list pointer
    movq %rax, 16(%rbx)  # Store element 1
    # Store element 2
    push %rbx  # Save list pointer
    mov $3, %rax
    pop %rbx  # Restore list pointer
    movq %rax, 24(%rbx)  # Store element 2
    mov %rbx, %rax  # List pointer
    mov %rax, -8(%rbp)  # store global a
    # Variable: b
    # Index expression: array[index]
    mov -8(%rbp), %rax  # load global a
    mov %rax, %rbx  # Save list pointer
    test %rbx, %rbx
    jz index_error_0  # Jump if null list
    mov $1, %rax
    mov %rax, %rcx  # Save index
    movq (%rbx), %rdx  # Load list size
    cmp %rdx, %rcx
    jge index_error_0  # Jump if index >= size
    test %rcx, %rcx
    js index_error_0  # Jump if index < 0
    imul $8, %rcx  # index * 8
    add $8, %rcx  # Add header offset
    add %rbx, %rcx  # base + offset
    movq (%rcx), %rax  # Load element value
    jmp index_done_0
index_error_0:
    # Print index error message and terminate
    mov $str_index_error, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $1, %rdi  # Exit code 1 for error
    call exit  # Terminate program
index_done_0:
    mov %rax, -16(%rbp)  # store global b
    # Call out(dtype(b))
    mov $dtype_unknown, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
