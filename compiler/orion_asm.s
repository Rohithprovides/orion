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
str_0: .string "=== Orion Input Function Demo ===\n"
str_1: .string "Enter your name: \n"
str_2: .string "Hello\n"
str_3: .string "Please enter your age:\n"
str_4: .string "You entered:\n"
str_5: .string "What's your favorite color? \n"
str_6: .string "What's your favorite food? \n"
str_7: .string "Summary:\n"
str_8: .string "Name:\n"
str_9: .string "Age:\n"
str_10: .string "Favorite color:\n"
str_11: .string "Favorite food:\n"

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

main:
    push %rbp
    mov %rsp, %rbp
    sub $64, %rsp
    # Function 'main' defined in scope ''
    # Explicit call to main()
    # Executing function call: main
    # Call out() with string
    mov $str_0, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Variable: name
    # input() function call
    mov $str_1, %rdi  # Prompt string
    call orion_input_prompt  # Display prompt and read input
    # String address returned in %rax
    mov %rax, -8(%rbp)  # store local name
    # Call out() with string
    mov $str_2, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out() with variable: name (type: unknown)
    mov -8(%rbp), %rsi
    mov $format_str, %rdi
    call printf
    # Call out() with string
    mov $str_3, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Variable: age
    # input() function call
    call orion_input  # Read input from stdin
    # String address returned in %rax
    mov %rax, -16(%rbp)  # store local age
    # Call out() with string
    mov $str_4, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out() with variable: age (type: unknown)
    mov -16(%rbp), %rsi
    mov $format_str, %rdi
    call printf
    # Variable: favorite_color
    # input() function call
    mov $str_5, %rdi  # Prompt string
    call orion_input_prompt  # Display prompt and read input
    # String address returned in %rax
    mov %rax, -24(%rbp)  # store local favorite_color
    # Variable: favorite_food
    # input() function call
    mov $str_6, %rdi  # Prompt string
    call orion_input_prompt  # Display prompt and read input
    # String address returned in %rax
    mov %rax, -32(%rbp)  # store local favorite_food
    # Call out() with string
    mov $str_7, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out() with string
    mov $str_8, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out() with variable: name (type: unknown)
    mov -8(%rbp), %rsi
    mov $format_str, %rdi
    call printf
    # Call out() with string
    mov $str_9, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out() with variable: age (type: unknown)
    mov -16(%rbp), %rsi
    mov $format_str, %rdi
    call printf
    # Call out() with string
    mov $str_10, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out() with variable: favorite_color (type: unknown)
    mov -24(%rbp), %rsi
    mov $format_str, %rdi
    call printf
    # Call out() with string
    mov $str_11, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out() with variable: favorite_food (type: unknown)
    mov -32(%rbp), %rsi
    mov $format_str, %rdi
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
