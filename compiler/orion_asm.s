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
str_0: .string "What is your name? \n"
str_1: .string "How old are you? \n"
str_2: .string "Hello \n"
str_3: .string "! You are \n"
str_4: .string " years old.\n"
str_5: .string "Welcome to Orion, \n"
str_6: .string "!\n"

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
    # Function 'main' defined in scope ''
    # Explicit call to main()
    # Executing function call: main
    # Variable: name
    # input() function call
    mov $str_0, %rdi  # Prompt string
    call orion_input_prompt  # Display prompt and read input
    # String address returned in %rax
    mov %rax, -8(%rbp)  # store local name
    # Variable: age
    # input() function call
    mov $str_1, %rdi  # Prompt string
    call orion_input_prompt  # Display prompt and read input
    # String address returned in %rax
    mov %rax, -16(%rbp)  # store local age
    # Call out() with interpolated string
    # Interpolated string - proper implementation
    # Multiple parts - simplified concatenation
    mov $0, %r12  # Initialize result string to null
    # Process part 0
    # Text part 0: "Hello "
    mov $str_2, %rax
    mov %rax, %rdi
    call string_to_string  # Copy string literal
    mov %rax, %r12  # Store first part
    # Process part 1
    # Expression part 1
    mov -8(%rbp), %rax  # load local name
    mov %rax, %rdi
    call string_to_string
    # Concatenate with previous result
    push %rax  # Save current part
    sub $16, %rsp  # Allocate space for 2 pointers
    mov %r12, 0(%rsp)  # Store previous result
    mov 16(%rsp), %rdi  # Get current part from stack
    mov %rdi, 8(%rsp)  # Store current part
    mov %rsp, %rdi  # Array of 2 string pointers
    mov $2, %rsi  # Number of parts to concatenate
    call string_concat_parts
    add $16, %rsp  # Clean up array space
    add $8, %rsp  # Clean up saved part
    mov %rax, %r12  # Store new result
    # Process part 2
    # Text part 2: "! You are "
    mov $str_3, %rax
    mov %rax, %rdi
    call string_to_string  # Copy string literal
    # Concatenate with previous result
    push %rax  # Save current part
    sub $16, %rsp  # Allocate space for 2 pointers
    mov %r12, 0(%rsp)  # Store previous result
    mov 16(%rsp), %rdi  # Get current part from stack
    mov %rdi, 8(%rsp)  # Store current part
    mov %rsp, %rdi  # Array of 2 string pointers
    mov $2, %rsi  # Number of parts to concatenate
    call string_concat_parts
    add $16, %rsp  # Clean up array space
    add $8, %rsp  # Clean up saved part
    mov %rax, %r12  # Store new result
    # Process part 3
    # Expression part 3
    mov -16(%rbp), %rax  # load local age
    mov %rax, %rdi
    call string_to_string
    # Concatenate with previous result
    push %rax  # Save current part
    sub $16, %rsp  # Allocate space for 2 pointers
    mov %r12, 0(%rsp)  # Store previous result
    mov 16(%rsp), %rdi  # Get current part from stack
    mov %rdi, 8(%rsp)  # Store current part
    mov %rsp, %rdi  # Array of 2 string pointers
    mov $2, %rsi  # Number of parts to concatenate
    call string_concat_parts
    add $16, %rsp  # Clean up array space
    add $8, %rsp  # Clean up saved part
    mov %rax, %r12  # Store new result
    # Process part 4
    # Text part 4: " years old."
    mov $str_4, %rax
    mov %rax, %rdi
    call string_to_string  # Copy string literal
    # Concatenate with previous result
    push %rax  # Save current part
    sub $16, %rsp  # Allocate space for 2 pointers
    mov %r12, 0(%rsp)  # Store previous result
    mov 16(%rsp), %rdi  # Get current part from stack
    mov %rdi, 8(%rsp)  # Store current part
    mov %rsp, %rdi  # Array of 2 string pointers
    mov $2, %rsi  # Number of parts to concatenate
    call string_concat_parts
    add $16, %rsp  # Clean up array space
    add $8, %rsp  # Clean up saved part
    mov %rax, %r12  # Store new result
    mov %r12, %rax  # Move result to return register
    # Multiple parts concatenation complete
    mov %rax, %rsi  # String pointer from interpolation result
    mov $format_str, %rdi  # Use string format
    xor %rax, %rax
    call printf
    # Call out() with interpolated string
    # Interpolated string - proper implementation
    # Multiple parts - simplified concatenation
    mov $0, %r12  # Initialize result string to null
    # Process part 0
    # Text part 0: "Welcome to Orion, "
    mov $str_5, %rax
    mov %rax, %rdi
    call string_to_string  # Copy string literal
    mov %rax, %r12  # Store first part
    # Process part 1
    # Expression part 1
    mov -8(%rbp), %rax  # load local name
    mov %rax, %rdi
    call string_to_string
    # Concatenate with previous result
    push %rax  # Save current part
    sub $16, %rsp  # Allocate space for 2 pointers
    mov %r12, 0(%rsp)  # Store previous result
    mov 16(%rsp), %rdi  # Get current part from stack
    mov %rdi, 8(%rsp)  # Store current part
    mov %rsp, %rdi  # Array of 2 string pointers
    mov $2, %rsi  # Number of parts to concatenate
    call string_concat_parts
    add $16, %rsp  # Clean up array space
    add $8, %rsp  # Clean up saved part
    mov %rax, %r12  # Store new result
    # Process part 2
    # Text part 2: "!"
    mov $str_6, %rax
    mov %rax, %rdi
    call string_to_string  # Copy string literal
    # Concatenate with previous result
    push %rax  # Save current part
    sub $16, %rsp  # Allocate space for 2 pointers
    mov %r12, 0(%rsp)  # Store previous result
    mov 16(%rsp), %rdi  # Get current part from stack
    mov %rdi, 8(%rsp)  # Store current part
    mov %rsp, %rdi  # Array of 2 string pointers
    mov $2, %rsi  # Number of parts to concatenate
    call string_concat_parts
    add $16, %rsp  # Clean up array space
    add $8, %rsp  # Clean up saved part
    mov %rax, %r12  # Store new result
    mov %r12, %rax  # Move result to return register
    # Multiple parts concatenation complete
    mov %rax, %rsi  # String pointer from interpolation result
    mov $format_str, %rdi  # Use string format
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
