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
str_0: .string "x is greater than 5\n"
str_1: .string "x is 5 or less\n"

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
    # Variable: x
    mov $2, %rax
    mov %rax, -8(%rbp)  # store global x
    # Integer binary operation
    mov -8(%rbp), %rax  # load global x
    push %rax
    mov $5, %rax
    pop %rbx
    cmp %rax, %rbx
    setge %al
    movzx %al, %rax
    test %rax, %rax
    jz else_0
    # Call out() with string
    mov $str_0, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    jmp end_if_0
else_0:
    # Call out() with string
    mov $str_1, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
end_if_0:
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
