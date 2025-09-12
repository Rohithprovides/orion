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
str_0: .string "Length:\n"
str_1: .string "After append:\n"
str_2: .string "Last element:\n"
str_3: .string "Combined length:\n"
str_4: .string "Repeated length:\n"
str_5: .string "Core features working!\n"
str_6: .string "Length:\n"
str_7: .string "After append:\n"
str_8: .string "Last element:\n"
str_9: .string "Combined length:\n"
str_10: .string "Repeated length:\n"
str_11: .string "Core features working!\n"

.section .text
.global main
.extern printf
.extern malloc
.extern free
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
    # Function defined: main
    # Explicit call to main()
    # Executing function call: main
    # Variable: numbers
    # Enhanced list literal with 3 elements
    # Allocating temporary array for 3 elements
    mov $24, %rdi
    call malloc  # Allocate temporary array
    mov %rax, %r12  # Save temp array pointer in %r12
    # Evaluating element 0
    push %r12  # Save temp array pointer
    mov $10, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 0(%r12)  # Store in temp array
    # Evaluating element 1
    push %r12  # Save temp array pointer
    mov $20, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 8(%r12)  # Store in temp array
    # Evaluating element 2
    push %r12  # Save temp array pointer
    mov $30, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 16(%r12)  # Store in temp array
    mov %r12, %rdi  # Temp array pointer
    mov $3, %rsi  # Element count
    call list_from_data  # Create list from data
    push %rax  # Save list pointer
    mov %r12, %rdi  # Temp array pointer
    call free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, -8(%rbp)  # store local numbers
    # Call out() with string
    mov $str_0, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # len() function call
    mov -8(%rbp), %rax  # load local numbers
    mov %rax, %rdi  # List pointer as argument
    call list_len  # Get list length
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # append() function call
    mov -8(%rbp), %rax  # load local numbers
    mov %rax, %rdi  # List pointer as first argument
    push %rdi  # Save list pointer
    mov $40, %rax
    mov %rax, %rsi  # Element value as second argument
    pop %rdi  # Restore list pointer
    call list_append  # Append element to list
    # Call out() with string
    mov $str_1, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # len() function call
    mov -8(%rbp), %rax  # load local numbers
    mov %rax, %rdi  # List pointer as argument
    call list_len  # Get list length
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
    # Enhanced index expression with negative indexing support
    mov -8(%rbp), %rax  # load local numbers
    mov %rax, %rdi  # List pointer as first argument
    mov $1, %rax
    neg %rax
    mov %rax, %rsi  # Index as second argument
    call list_get  # Get element with bounds checking
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Variable: more
    # Enhanced list literal with 2 elements
    # Allocating temporary array for 2 elements
    mov $16, %rdi
    call malloc  # Allocate temporary array
    mov %rax, %r12  # Save temp array pointer in %r12
    # Evaluating element 0
    push %r12  # Save temp array pointer
    mov $50, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 0(%r12)  # Store in temp array
    # Evaluating element 1
    push %r12  # Save temp array pointer
    mov $60, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 8(%r12)  # Store in temp array
    mov %r12, %rdi  # Temp array pointer
    mov $2, %rsi  # Element count
    call list_from_data  # Create list from data
    push %rax  # Save list pointer
    mov %r12, %rdi  # Temp array pointer
    call free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, -16(%rbp)  # store local more
    # Variable: combined
    # List concatenation: list + list
    mov -8(%rbp), %rax  # load local numbers
    mov %rax, %rdi  # First list as first argument
    push %rdi  # Save first list
    mov -16(%rbp), %rax  # load local more
    mov %rax, %rsi  # Second list as second argument
    pop %rdi  # Restore first list
    call list_concat  # Concatenate lists
    mov %rax, -24(%rbp)  # store local combined
    # Call out() with string
    mov $str_3, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # len() function call
    mov -24(%rbp), %rax  # load local combined
    mov %rax, %rdi  # List pointer as argument
    call list_len  # Get list length
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Variable: repeated
    # List repetition: n * list
    mov $2, %rax
    mov %rax, %rsi  # Repeat count as second argument
    push %rsi  # Save repeat count
    # Enhanced list literal with 1 elements
    # Allocating temporary array for 1 elements
    mov $8, %rdi
    call malloc  # Allocate temporary array
    mov %rax, %r12  # Save temp array pointer in %r12
    # Evaluating element 0
    push %r12  # Save temp array pointer
    mov $100, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 0(%r12)  # Store in temp array
    mov %r12, %rdi  # Temp array pointer
    mov $1, %rsi  # Element count
    call list_from_data  # Create list from data
    push %rax  # Save list pointer
    mov %r12, %rdi  # Temp array pointer
    call free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, %rdi  # List as first argument
    pop %rsi  # Restore repeat count
    call list_repeat  # Repeat list
    mov %rax, -32(%rbp)  # store local repeated
    # Call out() with string
    mov $str_4, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # len() function call
    mov -32(%rbp), %rax  # load local repeated
    mov %rax, %rdi  # List pointer as argument
    call list_len  # Get list length
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Call out() with string
    mov $str_5, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Calling user-defined main() function
    # Executing function call: main
    # Variable: numbers
    # Enhanced list literal with 3 elements
    # Allocating temporary array for 3 elements
    mov $24, %rdi
    call malloc  # Allocate temporary array
    mov %rax, %r12  # Save temp array pointer in %r12
    # Evaluating element 0
    push %r12  # Save temp array pointer
    mov $10, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 0(%r12)  # Store in temp array
    # Evaluating element 1
    push %r12  # Save temp array pointer
    mov $20, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 8(%r12)  # Store in temp array
    # Evaluating element 2
    push %r12  # Save temp array pointer
    mov $30, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 16(%r12)  # Store in temp array
    mov %r12, %rdi  # Temp array pointer
    mov $3, %rsi  # Element count
    call list_from_data  # Create list from data
    push %rax  # Save list pointer
    mov %r12, %rdi  # Temp array pointer
    call free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, -40(%rbp)  # store local numbers
    # Call out() with string
    mov $str_6, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # len() function call
    mov -40(%rbp), %rax  # load local numbers
    mov %rax, %rdi  # List pointer as argument
    call list_len  # Get list length
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # append() function call
    mov -40(%rbp), %rax  # load local numbers
    mov %rax, %rdi  # List pointer as first argument
    push %rdi  # Save list pointer
    mov $40, %rax
    mov %rax, %rsi  # Element value as second argument
    pop %rdi  # Restore list pointer
    call list_append  # Append element to list
    # Call out() with string
    mov $str_7, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # len() function call
    mov -40(%rbp), %rax  # load local numbers
    mov %rax, %rdi  # List pointer as argument
    call list_len  # Get list length
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Call out() with string
    mov $str_8, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # Enhanced index expression with negative indexing support
    mov -40(%rbp), %rax  # load local numbers
    mov %rax, %rdi  # List pointer as first argument
    mov $1, %rax
    neg %rax
    mov %rax, %rsi  # Index as second argument
    call list_get  # Get element with bounds checking
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Variable: more
    # Enhanced list literal with 2 elements
    # Allocating temporary array for 2 elements
    mov $16, %rdi
    call malloc  # Allocate temporary array
    mov %rax, %r12  # Save temp array pointer in %r12
    # Evaluating element 0
    push %r12  # Save temp array pointer
    mov $50, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 0(%r12)  # Store in temp array
    # Evaluating element 1
    push %r12  # Save temp array pointer
    mov $60, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 8(%r12)  # Store in temp array
    mov %r12, %rdi  # Temp array pointer
    mov $2, %rsi  # Element count
    call list_from_data  # Create list from data
    push %rax  # Save list pointer
    mov %r12, %rdi  # Temp array pointer
    call free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, -48(%rbp)  # store local more
    # Variable: combined
    # List concatenation: list + list
    mov -40(%rbp), %rax  # load local numbers
    mov %rax, %rdi  # First list as first argument
    push %rdi  # Save first list
    mov -48(%rbp), %rax  # load local more
    mov %rax, %rsi  # Second list as second argument
    pop %rdi  # Restore first list
    call list_concat  # Concatenate lists
    mov %rax, -56(%rbp)  # store local combined
    # Call out() with string
    mov $str_9, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # len() function call
    mov -56(%rbp), %rax  # load local combined
    mov %rax, %rdi  # List pointer as argument
    call list_len  # Get list length
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Variable: repeated
    # List repetition: n * list
    mov $2, %rax
    mov %rax, %rsi  # Repeat count as second argument
    push %rsi  # Save repeat count
    # Enhanced list literal with 1 elements
    # Allocating temporary array for 1 elements
    mov $8, %rdi
    call malloc  # Allocate temporary array
    mov %rax, %r12  # Save temp array pointer in %r12
    # Evaluating element 0
    push %r12  # Save temp array pointer
    mov $100, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 0(%r12)  # Store in temp array
    mov %r12, %rdi  # Temp array pointer
    mov $1, %rsi  # Element count
    call list_from_data  # Create list from data
    push %rax  # Save list pointer
    mov %r12, %rdi  # Temp array pointer
    call free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, %rdi  # List as first argument
    pop %rsi  # Restore repeat count
    call list_repeat  # Repeat list
    mov %rax, -64(%rbp)  # store local repeated
    # Call out() with string
    mov $str_10, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    # len() function call
    mov -64(%rbp), %rax  # load local repeated
    mov %rax, %rdi  # List pointer as argument
    call list_len  # Get list length
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Call out() with string
    mov $str_11, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
