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

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Variable: a
    mov $5, %rax
    mov %rax, -8(%rbp)  # store global a
    # Call out() with variable: a (type: int)
    mov -8(%rbp), %rsi
    mov $format_int, %rdi
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
