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
    # Variable: x
    # Enhanced list literal with 3 elements
    # Allocating temporary array for 3 elements
    mov $24, %rdi
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
    # Evaluating element 2
    push %r12  # Save temp array pointer
    mov $3, %rax
    pop %r12  # Restore temp array pointer
    movq %rax, 16(%r12)  # Store in temp array
    mov %r12, %rdi  # Temp array pointer
    mov $3, %rsi  # Element count
    call list_from_data  # Create list from data
    push %rax  # Save list pointer
    mov %r12, %rdi  # Temp array pointer
    call free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, -8(%rbp)  # store local x
    # Enhanced index expression with negative indexing support
    mov -8(%rbp), %rax  # load local x
    mov %rax, %rdi  # List pointer as first argument
    mov $0, %rax
    mov %rax, %rsi  # Index as second argument
    call list_get  # Get element with bounds checking
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Index assignment: list[index] = value
    mov -8(%rbp), %rax  # load local x
    mov %rax, %r12  # Save list pointer in %r12
    mov $0, %rax
    mov %rax, %r13  # Save index in %r13
    mov $99, %rax
    mov %rax, %rdx  # Value in %rdx (third argument)
    mov %r12, %rdi  # List pointer as first argument
    mov %r13, %rsi  # Index as second argument
    # Value already in %rdx as third argument
    call list_set  # Set list[index] = value
    # Enhanced index expression with negative indexing support
    mov -8(%rbp), %rax  # load local x
    mov %rax, %rdi  # List pointer as first argument
    mov $0, %rax
    mov %rax, %rsi  # Index as second argument
    call list_get  # Get element with bounds checking
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    # Index assignment: list[index] = value
    mov -8(%rbp), %rax  # load local x
    mov %rax, %r12  # Save list pointer in %r12
    mov $1, %rax
    neg %rax
    mov %rax, %r13  # Save index in %r13
    mov $77, %rax
    mov %rax, %rdx  # Value in %rdx (third argument)
    mov %r12, %rdi  # List pointer as first argument
    mov %r13, %rsi  # Index as second argument
    # Value already in %rdx as third argument
    call list_set  # Set list[index] = value
    # Enhanced index expression with negative indexing support
    mov -8(%rbp), %rax  # load local x
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
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
