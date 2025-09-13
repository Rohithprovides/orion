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
str_0: .string "Score is:\n"
str_1: .string "In first condition: >= 90\n"
str_2: .string "In second condition: >= 80\n"
str_3: .string "In else condition: < 80\n"

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

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Variable: score
    mov $85, %rax
    mov %rax, -8(%rbp)  # store global score
    # Call out() with string
    mov $str_0, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out() with variable: score (type: int)
    mov -8(%rbp), %rsi
    mov $format_int, %rdi
    call printf
    # Integer binary operation
    mov -8(%rbp), %rax  # load global score
    push %rax
    mov $90, %rax
    pop %rbx
    cmp %rax, %rbx
    setge %al
    movzx %al, %rax
    test %rax, %rax
    jz else_0
    # Call out() with string
    mov $str_1, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    jmp end_if_0
else_0:
    # Integer binary operation
    mov -8(%rbp), %rax  # load global score
    push %rax
    mov $80, %rax
    pop %rbx
    cmp %rax, %rbx
    setge %al
    movzx %al, %rax
    test %rax, %rax
    jz else_1
    # Call out() with string
    mov $str_2, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    jmp end_if_1
else_1:
    # Call out() with string
    mov $str_3, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
end_if_1:
end_if_0:
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
