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
str_0: .string "alice"
str_1: .string "alice"
str_2: .string "This works correctly now!"
str_3: .string "Integer comparisons work too!"

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
    # Variable: x
    mov $str_0, %rax
    mov %rax, -8(%rbp)  # store global x
    # String comparison operation
    mov -8(%rbp), %rax  # load global x
    mov %rax, %rdi  # First string as first argument
    push %rdi  # Save first string
    mov $str_1, %rax
    mov %rax, %rsi  # Second string as second argument
    pop %rdi  # Restore first string
    call strcmp  # Compare strings
    test %rax, %rax
    je seq_true_1
    mov $str_false, %rax
    jmp seq_done_1
seq_true_1:
    mov $str_true, %rax
seq_done_1:
    test %rax, %rax
    jz else_0
    # Call out() with string
    mov $str_2, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    jmp end_if_0
else_0:
end_if_0:
    # Variable: y
    mov $10, %rax
    mov %rax, -16(%rbp)  # store global y
    # Integer binary operation
    mov -16(%rbp), %rax  # load global y
    push %rax
    mov $5, %rax
    pop %rbx
    cmp %rax, %rbx
    setg %al
    movzx %al, %rax
    test %rax, %rax
    jz else_2
    # Call out() with string
    mov $str_3, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    jmp end_if_2
else_2:
end_if_2:
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
