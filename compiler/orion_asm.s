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

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # range() function call
    mov $10, %rax
    mov %rax, %rdi  # Stop value as argument
    call range_new_stop  # Create range with stop only
    mov %rax, %r12  # Store iterable pointer
    mov $0, %r13    # Initialize index
    # For-in loop over range object
    mov %r12, %rdi  # Range pointer
    call range_len  # Get range length
    mov %rax, %r14  # Store range length
forin_loop_0:
    cmp %r14, %r13
    jge forin_end_0
    mov %r12, %rdi  # Range pointer
    mov %r13, %rsi  # Index
    call range_get   # Get element at index
    mov %rax, -8(%rbp)  # i = %rax (type: int)
    # Integer binary operation
    mov -8(%rbp), %rax  # load global i
    push %rax
    mov $3, %rax
    pop %rbx
    cmp %rax, %rbx
    sete %al
    movzx %al, %rax
    test %rax, %rax
    jz else_1
    jmp forin_end_0
    jmp end_if_1
else_1:
end_if_1:
    # Integer binary operation
    mov -8(%rbp), %rax  # load global i
    push %rax
    mov $1, %rax
    pop %rbx
    cmp %rax, %rbx
    sete %al
    movzx %al, %rax
    test %rax, %rax
    jz else_2
    jmp forin_loop_0
    jmp end_if_2
else_2:
end_if_2:
    # Integer binary operation
    mov -8(%rbp), %rax  # load global i
    push %rax
    mov $2, %rax
    pop %rbx
    cmp %rax, %rbx
    sete %al
    movzx %al, %rax
    test %rax, %rax
    jz else_3
    # pass statement
    jmp end_if_3
else_3:
end_if_3:
    # Call out() with variable: i (type: int)
    mov -8(%rbp), %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    inc %r13
    jmp forin_loop_0
forin_end_0:
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
