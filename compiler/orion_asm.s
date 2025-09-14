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
str_0: .string "Number: "

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
    # Function prologue: setting up parameters
    # Variable: numbers
    # Enhanced list literal with 3 elements
    # Allocating temporary array for 3 elements
    mov $24, %rdi
    call orion_malloc  # Allocate temporary array
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
    call orion_free  # Free temporary array
    pop %rax  # Restore list pointer
    mov %rax, -8(%rbp)  # store local numbers
    mov -8(%rbp), %rax  # load local numbers
    mov %rax, %r12  # Store iterable pointer
    mov $0, %r13    # Initialize index
    # For-in loop over list object (default)
    mov (%r12), %r14  # Load list length
forin_loop_0:
    cmp %r14, %r13
    jge forin_end_0
    mov %r12, %rdi  # List pointer
    mov %r13, %rsi  # Index
    call list_get   # Get element at index
    mov %rax, -16(%rbp)  # num = %rax (type: int)
    # Integer binary operation
    mov $str_0, %rax
    push %rax
    # str() type conversion function call
    mov -16(%rbp), %rax  # load local num
    mov %rax, %rdi  # int variable
    call __orion_int_to_string
    pop %rbx
    add %rbx, %rax
    # Call out() with expression result
    mov %rax, %rsi
    mov $format_int, %rdi
    xor %rax, %rax
    call printf
    inc %r13
    jmp forin_loop_0
forin_end_0:
    mov $0, %rax
    add $64, %rsp
    pop %rbp
    ret
