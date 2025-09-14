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
str_0: .string "Test 1: len(range(5)) ="
str_1: .string "Test 2: len(range(2, 8)) ="
str_2: .string "Test 3: len(range(1, 10, 2)) ="
str_3: .string "Test 4: for i in range(3) with break:"
str_4: .string "All basic tests completed!"

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
    # Call out() with string
    mov $str_0, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # len() function call
    # range() function call
    mov $5, %rax
    mov %rax, %rdi  # Stop value as argument
    call range_new_stop  # Create range with stop only
    mov %rax, %rdi  # Range pointer as argument
    call range_len  # Get range length
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Call out() with string
    mov $str_1, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # len() function call
    # range() function call
    mov $2, %rax
    mov %rax, %rdi  # Start value as first argument
    push %rdi  # Save start value
    mov $8, %rax
    mov %rax, %rsi  # Stop value as second argument
    pop %rdi  # Restore start value
    call range_new_start_stop  # Create range with start and stop
    mov %rax, %rdi  # Range pointer as argument
    call range_len  # Get range length
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Call out() with string
    mov $str_2, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # len() function call
    # range() function call
    mov $1, %rax
    mov %rax, %rdi  # Start value as first argument
    push %rdi  # Save start value
    mov $10, %rax
    mov %rax, %rsi  # Stop value as second argument
    push %rsi  # Save stop value
    mov $2, %rax
    mov %rax, %rdx  # Step value as third argument
    pop %rsi  # Restore stop value
    pop %rdi  # Restore start value
    call range_new  # Create range with start, stop, and step
    mov %rax, %rdi  # Range pointer as argument
    call range_len  # Get range length
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Call out() with string
    mov $str_3, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # range() function call
    mov $3, %rax
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
    mov %rax, -8(%rbp)  # i = %rax
    # Call out() with variable: i (type: unknown)
    mov -8(%rbp), %rsi
    mov $format_str, %rdi
    call printf
    # Integer binary operation
    mov -8(%rbp), %rax  # load global i
    push %rax
    mov $1, %rax
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
    inc %r13
    jmp forin_loop_0
forin_end_0:
    # Call out() with string
    mov $str_4, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
