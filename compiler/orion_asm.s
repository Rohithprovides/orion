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
.extern orion_malloc
.extern orion_free
.extern exit
.extern fmod
.extern pow
.extern strcmp
.extern list_new
.extern list_from_data
.extern list_len
.extern list_get
.extern list_set
.extern list_append
.extern list_pop
.extern list_insert
.extern list_concat
.extern list_repeat
.extern list_extend
.extern orion_input
.extern orion_input_prompt
.extern int_to_string
.extern float_to_string
.extern bool_to_string
.extern string_to_string
.extern string_concat_parts


multiply:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp  # Allocate stack space for local variables
    # Setting up function parameters for multiply
    mov %rdi, -8(%rbp)  # Parameter x
    mov %rsi, -16(%rbp)  # Parameter y
    mov %rdx, -24(%rbp)  # Parameter z
    # Integer binary operation
    # Integer binary operation
    mov -8(%rbp), %rax  # load local x
    push %rax
    mov -16(%rbp), %rax  # load local y
    pop %rbx
    imul %rbx, %rax
    push %rax
    mov -24(%rbp), %rax  # load local z
    pop %rbx
    imul %rbx, %rax
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    add $64, %rsp  # Restore stack space
    pop %rbp
    ret
main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Function 'multiply' defined in scope ''
    # User-defined function call: multiply
    # Preparing argument 0
    mov $2, %rax
    mov %rax, %rdi  # Arg 0 to %rdi
    # Preparing argument 1
    mov $3, %rax
    mov %rax, %rsi  # Arg 1 to %rsi
    # Preparing argument 2
    mov $4, %rax
    mov %rax, %rdx  # Arg 2 to %rdx
    call multiply
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
