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

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Variable: a
    mov $5, %rax
    mov %rax, -8(%rbp)  # store global a
    # Variable: b
    mov $2, %rax
    mov %rax, -16(%rbp)  # store global b
    # Integer binary operation
    mov -8(%rbp), %rax  # load global a
    push %rax
    mov -16(%rbp), %rax  # load global b
    pop %rbx
    add %rbx, %rax
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Integer binary operation
    mov -8(%rbp), %rax  # load global a
    push %rax
    mov -16(%rbp), %rax  # load global b
    pop %rbx
    sub %rax, %rbx
    mov %rbx, %rax
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Integer binary operation
    mov -8(%rbp), %rax  # load global a
    push %rax
    mov -16(%rbp), %rax  # load global b
    pop %rbx
    imul %rbx, %rax
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Integer binary operation
    mov -8(%rbp), %rax  # load global a
    push %rax
    mov -16(%rbp), %rax  # load global b
    pop %rbx
    mov %rax, %rcx
    mov %rbx, %rax
    xor %rdx, %rdx
    idiv %rcx
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Integer binary operation
    mov -8(%rbp), %rax  # load global a
    push %rax
    mov -16(%rbp), %rax  # load global b
    pop %rbx
    mov %rbx, %rcx  # base
    mov %rax, %rdx  # exponent
    mov $1, %rax    # result = 1
power_loop:
    test %rdx, %rdx
    jz power_done
    imul %rcx, %rax
    dec %rdx
    jmp power_loop
power_done:
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Integer binary operation
    mov -8(%rbp), %rax  # load global a
    push %rax
    mov -16(%rbp), %rax  # load global b
    pop %rbx
    mov %rax, %rcx
    mov %rbx, %rax
    xor %rdx, %rdx
    idiv %rcx
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Integer binary operation
    mov -8(%rbp), %rax  # load global a
    push %rax
    mov -16(%rbp), %rax  # load global b
    pop %rbx
    mov %rax, %rcx
    mov %rbx, %rax
    xor %rdx, %rdx
    idiv %rcx
    mov %rdx, %rax
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
