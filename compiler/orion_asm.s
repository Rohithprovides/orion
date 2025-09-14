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
str_0: .string "=== Testing Built-in Type Conversion Functions ==="
str_1: .string "Testing str():"
str_2: .string "Testing int():"
str_3: .string "Testing flt():"
str_4: .string "Type conversion tests completed!"
float_0: .quad 4614253070214989087
float_1: .quad 4614253070214989087

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
    # Function 'main' defined in scope ''
    # Auto-executing main() function
    # Executing function call: main
    # Call out() with string
    mov $str_0, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out() with string
    mov $str_1, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Variable: age
    mov $25, %rax
    mov %rax, -8(%rbp)  # store local age
    # Call out() with str() result
    # str() type conversion function call
    mov -8(%rbp), %rax  # load local age
    mov %rax, %rdi  # int variable
    call __orion_int_to_string
    mov %rax, %rsi  # String pointer as argument
    mov $format_str, %rdi  # Use string format
    xor %rax, %rax
    call printf
    # Call out() with string
    mov $str_2, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out() with str() result
    # str() type conversion function call
    # int() type conversion function call
    mov $42, %rax
    # Int to int conversion (identity)
    mov %rax, %rdi  # complex expression argument
    call __orion_int_to_string  # Default to int conversion
    mov %rax, %rsi  # String pointer as argument
    mov $format_str, %rdi  # Use string format
    xor %rax, %rax
    call printf
    # Variable: pi
    # Float: 3.14
    movq float_0(%rip), %rax
    mov %rax, -16(%rbp)  # store local pi
    # Call out() with str() result
    # str() type conversion function call
    # int() type conversion function call
    mov -16(%rbp), %rax  # load local pi
    movq %rax, %xmm0  # float variable
    call __orion_float_to_int
    mov %rax, %rdi  # complex expression argument
    call __orion_int_to_string  # Default to int conversion
    mov %rax, %rsi  # String pointer as argument
    mov $format_str, %rdi  # Use string format
    xor %rax, %rax
    call printf
    # Call out() with string
    mov $str_3, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Call out() with str() result
    # str() type conversion function call
    # flt() type conversion function call
    mov $42, %rax
    mov %rax, %rdi  # int argument
    call __orion_int_to_float
    mov %rax, %rdi  # complex expression argument
    call __orion_int_to_string  # Default to int conversion
    mov %rax, %rsi  # String pointer as argument
    mov $format_str, %rdi  # Use string format
    xor %rax, %rax
    call printf
    # Call out() with str() result
    # str() type conversion function call
    # flt() type conversion function call
    # Float: 3.14
    movq float_1(%rip), %rax
    # Float to float conversion (identity)
    mov %rax, %rdi  # complex expression argument
    call __orion_int_to_string  # Default to int conversion
    mov %rax, %rsi  # String pointer as argument
    mov $format_str, %rdi  # Use string format
    xor %rax, %rax
    call printf
    # Call out() with string
    mov $str_4, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
