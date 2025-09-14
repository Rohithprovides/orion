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
str_0: .string "hello"
str_1: .string "world"
str_2: .string "hello"
str_3: .string "Testing all string comparison operators:"
str_4: .string "a == c: TRUE"
str_5: .string "a == c: FALSE"
str_6: .string "a != b: TRUE"
str_7: .string "a != b: FALSE"
str_8: .string "a < b: TRUE"
str_9: .string "a < b: FALSE"
str_10: .string "b > a: TRUE"
str_11: .string "b > a: FALSE"

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
    mov $str_0, %rax
    mov %rax, -8(%rbp)  # store global a
    # Variable: b
    mov $str_1, %rax
    mov %rax, -16(%rbp)  # store global b
    # Variable: c
    mov $str_2, %rax
    mov %rax, -24(%rbp)  # store global c
    # Call out() with string
    mov $str_3, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # String comparison operation
    mov -8(%rbp), %rax  # load global a
    mov %rax, %rdi  # First string as first argument
    push %rdi  # Save first string
    mov -24(%rbp), %rax  # load global c
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
    mov $str_4, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    jmp end_if_0
else_0:
    # Call out() with string
    mov $str_5, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
end_if_0:
    # String comparison operation
    mov -8(%rbp), %rax  # load global a
    mov %rax, %rdi  # First string as first argument
    push %rdi  # Save first string
    mov -16(%rbp), %rax  # load global b
    mov %rax, %rsi  # Second string as second argument
    pop %rdi  # Restore first string
    call strcmp  # Compare strings
    test %rax, %rax
    jne sne_true_3
    mov $str_false, %rax
    jmp sne_done_3
sne_true_3:
    mov $str_true, %rax
sne_done_3:
    test %rax, %rax
    jz else_2
    # Call out() with string
    mov $str_6, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    jmp end_if_2
else_2:
    # Call out() with string
    mov $str_7, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
end_if_2:
    # String comparison operation
    mov -8(%rbp), %rax  # load global a
    mov %rax, %rdi  # First string as first argument
    push %rdi  # Save first string
    mov -16(%rbp), %rax  # load global b
    mov %rax, %rsi  # Second string as second argument
    pop %rdi  # Restore first string
    call strcmp  # Compare strings
    test %rax, %rax
    js slt_true_5
    mov $str_false, %rax
    jmp slt_done_5
slt_true_5:
    mov $str_true, %rax
slt_done_5:
    test %rax, %rax
    jz else_4
    # Call out() with string
    mov $str_8, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    jmp end_if_4
else_4:
    # Call out() with string
    mov $str_9, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
end_if_4:
    # String comparison operation
    mov -16(%rbp), %rax  # load global b
    mov %rax, %rdi  # First string as first argument
    push %rdi  # Save first string
    mov -8(%rbp), %rax  # load global a
    mov %rax, %rsi  # Second string as second argument
    pop %rdi  # Restore first string
    call strcmp  # Compare strings
    test %rax, %rax
    jg sgt_true_7
    mov $str_false, %rax
    jmp sgt_done_7
sgt_true_7:
    mov $str_true, %rax
sgt_done_7:
    test %rax, %rax
    jz else_6
    # Call out() with string
    mov $str_10, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    jmp end_if_6
else_6:
    # Call out() with string
    mov $str_11, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
end_if_6:
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
