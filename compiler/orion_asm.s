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
str_0: .string "Empty list created\n"

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
    # Function defined: main
    # Calling user-defined main() function
    # Executing function call: main
    # Variable: empty
    # List literal with 0 elements
    mov $8, %rdi  # Allocation size for empty list (just size header)
    call malloc  # Allocate memory for empty list
    mov %rax, %rbx  # Save list pointer
    movq $0, (%rbx)  # Store list size = 0
    mov %rbx, %rax  # Return list pointer
    mov %rax, -8(%rbp)  # store local empty
    # Call out() with string
    mov $str_0, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
