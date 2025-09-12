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
str_0: .string "All valid operations work\n"

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
    # Calling user-defined main() function
    # Executing function call: main
    # Variable: list1
    # Enhanced list literal with 2 elements
    # Allocating temporary array for 2 elements
    mov $16, %rdi
    call malloc  # Allocate temporary array
    mov %rax, %r12  # Save temp array pointer in %r12
    # Evaluating element 0
    push %r12  # Save temp array pointer
    mov $1, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 0(%r12)  # Store in temp array
    # Evaluating element 1
    push %r12  # Save temp array pointer
    mov $2, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 8(%r12)  # Store in temp array
    mov %r12, %rdi  # Temp array pointer
    mov $2, %rsi  # Element count
    call list_from_data  # Create list from data
    push %rax  # Save list pointer
    mov %r12, %rdi  # Temp array pointer
    call free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, -8(%rbp)  # store local list1
    # Variable: list2
    # Enhanced list literal with 2 elements
    # Allocating temporary array for 2 elements
    mov $16, %rdi
    call malloc  # Allocate temporary array
    mov %rax, %r12  # Save temp array pointer in %r12
    # Evaluating element 0
    push %r12  # Save temp array pointer
    mov $3, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 0(%r12)  # Store in temp array
    # Evaluating element 1
    push %r12  # Save temp array pointer
    mov $4, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 8(%r12)  # Store in temp array
    mov %r12, %rdi  # Temp array pointer
    mov $2, %rsi  # Element count
    call list_from_data  # Create list from data
    push %rax  # Save list pointer
    mov %r12, %rdi  # Temp array pointer
    call free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, -16(%rbp)  # store local list2
    # Variable: nested
    # List concatenation: list + list
    # List concatenation: list + list
    mov -8(%rbp), %rax  # load local list1
    mov %rax, %rdi  # First list as first argument
    push %rdi  # Save first list
    mov -16(%rbp), %rax  # load local list2
    mov %rax, %rsi  # Second list as second argument
    pop %rdi  # Restore first list
    call list_concat  # Concatenate lists
    mov %rax, %rdi  # First list as first argument
    push %rdi  # Save first list
    # Enhanced list literal with 2 elements
    # Allocating temporary array for 2 elements
    mov $16, %rdi
    call malloc  # Allocate temporary array
    mov %rax, %r12  # Save temp array pointer in %r12
    # Evaluating element 0
    push %r12  # Save temp array pointer
    mov $5, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 0(%r12)  # Store in temp array
    # Evaluating element 1
    push %r12  # Save temp array pointer
    mov $6, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 8(%r12)  # Store in temp array
    mov %r12, %rdi  # Temp array pointer
    mov $2, %rsi  # Element count
    call list_from_data  # Create list from data
    push %rax  # Save list pointer
    mov %r12, %rdi  # Temp array pointer
    call free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, %rsi  # Second list as second argument
    pop %rdi  # Restore first list
    call list_concat  # Concatenate lists
    mov %rax, -24(%rbp)  # store local nested
    # len() function call
    mov -24(%rbp), %rax  # load local nested
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
    # Enhanced list literal with 2 elements
    # Allocating temporary array for 2 elements
    mov $16, %rdi
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
    mov %r12, %rdi  # Temp array pointer
    mov $2, %rsi  # Element count
    call list_from_data  # Create list from data
    push %rax  # Save list pointer
    mov %r12, %rdi  # Temp array pointer
    call free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, %rdi  # List as first argument
    pop %rsi  # Restore repeat count
    call list_repeat  # Repeat list
    mov %rax, -32(%rbp)  # store local repeated
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
    mov $str_0, %rsi
    mov $format_str, %rdi
    xor %rax, %rax
    call printf
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
