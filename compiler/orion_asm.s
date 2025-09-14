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
str_0: .string "enter your name"
str_1: .string "tony"
str_2: .string "tony"
str_3: .string "stranger"

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
    # Variable: name
    # input() function call
    mov $str_0, %rdi  # Prompt string
    call orion_input_prompt  # Display prompt and read input
    # String address returned in %rax
    mov %rax, -8(%rbp)  # store global name
    # String comparison operation
    mov -8(%rbp), %rax  # load global name
    mov %rax, %rdi  # First string as first argument
    push %rdi  # Save first string
    mov $str_1, %rax
    mov %rax, %rsi  # Second string as second argument
    pop %rdi  # Restore first string
    call strcmp  # Compare strings
    cmp $0, %eax  # Compare strcmp result with 0
    sete %al      # Set %al to 1 if equal, 0 if not
    movzx %al, %rax  # Zero-extend to full register
    test %rax, %rax
    jz else_0
    # Call out() with string
    mov $str_2, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    jmp end_if_0
else_0:
    # Call out() with string
    mov $str_3, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
end_if_0:
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
