#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Orion-specific memory allocation wrappers to avoid symbol collision
void* orion_malloc(size_t size) {
    return malloc(size);
}

void orion_free(void* ptr) {
    free(ptr);
}

void* orion_realloc(void* ptr, size_t size) {
    return realloc(ptr, size);
}

// Enhanced list structure for dynamic operations
typedef struct {
    int64_t size;        // Current number of elements
    int64_t capacity;    // Total allocated space
    int64_t* data;       // Pointer to element array (8 bytes per element)
} OrionList;

// Create a new empty list with initial capacity
OrionList* list_new(int64_t initial_capacity) {
    if (initial_capacity < 4) initial_capacity = 4; // Minimum capacity
    
    OrionList* list = (OrionList*)orion_malloc(sizeof(OrionList));
    if (!list) {
        fprintf(stderr, "Error: Failed to allocate memory for list\n");
        exit(1);
    }
    
    list->size = 0;
    list->capacity = initial_capacity;
    list->data = (int64_t*)orion_malloc(sizeof(int64_t) * initial_capacity);
    if (!list->data) {
        fprintf(stderr, "Error: Failed to allocate memory for list data\n");
        exit(1);
    }
    
    return list;
}

// Create a list from existing data (used by list literals)
OrionList* list_from_data(int64_t* elements, int64_t count) {
    OrionList* list = list_new(count > 4 ? count : 4);
    list->size = count;
    
    // Copy elements
    for (int64_t i = 0; i < count; i++) {
        list->data[i] = elements[i];
    }
    
    return list;
}

// Get list length
int64_t list_len(OrionList* list) {
    if (!list) {
        fprintf(stderr, "Error: Cannot get length of null list\n");
        exit(1);
    }
    return list->size;
}

// Normalize negative index to positive (Python-style)
int64_t normalize_index(OrionList* list, int64_t index) {
    if (!list) {
        fprintf(stderr, "Error: Cannot normalize index on null list\n");
        exit(1);
    }
    
    if (index < 0) {
        index += list->size;
    }
    
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "Error: List index out of range\n");
        exit(1);
    }
    
    return index;
}

// Get element at index (supports negative indexing)
int64_t list_get(OrionList* list, int64_t index) {
    if (!list) {
        fprintf(stderr, "Error: Cannot access null list\n");
        exit(1);
    }
    
    index = normalize_index(list, index);
    return list->data[index];
}

// Set element at index (supports negative indexing)
void list_set(OrionList* list, int64_t index, int64_t value) {
    if (!list) {
        fprintf(stderr, "Error: Cannot modify null list\n");
        exit(1);
    }
    
    index = normalize_index(list, index);
    list->data[index] = value;
}

// Resize list capacity (internal function)
void list_resize(OrionList* list, int64_t new_capacity) {
    if (!list) return;
    
    // Protect against integer overflow
    if (new_capacity > INT64_MAX / sizeof(int64_t)) {
        fprintf(stderr, "Error: List capacity too large\n");
        exit(1);
    }
    
    int64_t* new_data = (int64_t*)orion_realloc(list->data, sizeof(int64_t) * new_capacity);
    if (!new_data) {
        fprintf(stderr, "Error: Failed to resize list\n");
        exit(1);
    }
    
    list->data = new_data;
    list->capacity = new_capacity;
}

// Append element to end of list
void list_append(OrionList* list, int64_t value) {
    if (!list) {
        fprintf(stderr, "Error: Cannot append to null list\n");
        exit(1);
    }
    
    // Resize if needed (double capacity)
    if (list->size >= list->capacity) {
        int64_t new_capacity = list->capacity * 2;
        list_resize(list, new_capacity);
    }
    
    list->data[list->size] = value;
    list->size++;
}

// Remove and return last element
int64_t list_pop(OrionList* list) {
    if (!list) {
        fprintf(stderr, "Error: Cannot pop from null list\n");
        exit(1);
    }
    
    if (list->size == 0) {
        fprintf(stderr, "Error: Cannot pop from empty list\n");
        exit(1);
    }
    
    list->size--;
    int64_t value = list->data[list->size];
    
    // Shrink capacity if list becomes much smaller (optional optimization)
    if (list->size < list->capacity / 4 && list->capacity > 8) {
        list_resize(list, list->capacity / 2);
    }
    
    return value;
}

// Insert element at specific index
void list_insert(OrionList* list, int64_t index, int64_t value) {
    if (!list) {
        fprintf(stderr, "Error: Cannot insert into null list\n");
        exit(1);
    }
    
    // Allow inserting at end (index == size)
    if (index < 0) {
        index += list->size;
    }
    if (index < 0 || index > list->size) {
        fprintf(stderr, "Error: Insert index out of range\n");
        exit(1);
    }
    
    // Resize if needed
    if (list->size >= list->capacity) {
        int64_t new_capacity = list->capacity * 2;
        list_resize(list, new_capacity);
    }
    
    // Shift elements to make room
    memmove(&list->data[index + 1], &list->data[index], 
            sizeof(int64_t) * (list->size - index));
    
    list->data[index] = value;
    list->size++;
}

// Concatenate two lists (returns new list)
OrionList* list_concat(OrionList* list1, OrionList* list2) {
    if (!list1 || !list2) {
        fprintf(stderr, "Error: Cannot concatenate null lists\n");
        exit(1);
    }
    
    int64_t total_size = list1->size + list2->size;
    OrionList* result = list_new(total_size);
    result->size = total_size;
    
    // Copy elements from both lists
    memcpy(result->data, list1->data, sizeof(int64_t) * list1->size);
    memcpy(&result->data[list1->size], list2->data, sizeof(int64_t) * list2->size);
    
    return result;
}

// Repeat list n times (returns new list)
OrionList* list_repeat(OrionList* list, int64_t count) {
    if (!list) {
        fprintf(stderr, "Error: Cannot repeat null list\n");
        exit(1);
    }
    
    if (count < 0) {
        fprintf(stderr, "Error: Cannot repeat list negative times\n");
        exit(1);
    }
    
    if (count == 0 || list->size == 0) {
        return list_new(4);
    }
    
    // Protect against overflow
    if (list->size > INT64_MAX / count) {
        fprintf(stderr, "Error: Repeated list would be too large\n");
        exit(1);
    }
    
    int64_t total_size = list->size * count;
    OrionList* result = list_new(total_size);
    result->size = total_size;
    
    // Copy the list data count times
    for (int64_t i = 0; i < count; i++) {
        memcpy(&result->data[i * list->size], list->data, sizeof(int64_t) * list->size);
    }
    
    return result;
}

// Extend list with elements from another list (modifies first list)
void list_extend(OrionList* list1, OrionList* list2) {
    if (!list1 || !list2) {
        fprintf(stderr, "Error: Cannot extend null lists\n");
        exit(1);
    }
    
    // Resize if needed
    int64_t new_size = list1->size + list2->size;
    if (new_size > list1->capacity) {
        int64_t new_capacity = list1->capacity;
        while (new_capacity < new_size) {
            new_capacity *= 2;
        }
        list_resize(list1, new_capacity);
    }
    
    // Copy elements from list2
    memcpy(&list1->data[list1->size], list2->data, sizeof(int64_t) * list2->size);
    list1->size = new_size;
}

// Print list for debugging (optional)
void list_print(OrionList* list) {
    if (!list) {
        printf("null\n");
        return;
    }
    
    printf("[");
    for (int64_t i = 0; i < list->size; i++) {
        if (i > 0) printf(", ");
        printf("%ld", list->data[i]);
    }
    printf("]\n");
}

// Input function - read a line from stdin
char* orion_input() {
    const int BUFFER_SIZE = 1024;
    char* buffer = (char*)orion_malloc(BUFFER_SIZE);
    if (!buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for input\n");
        exit(1);
    }
    
    // Read line from stdin
    if (!fgets(buffer, BUFFER_SIZE, stdin)) {
        // Handle EOF or error
        orion_free(buffer);
        buffer = (char*)orion_malloc(1);
        buffer[0] = '\0';
        return buffer;
    }
    
    // Remove trailing newline if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    // Resize buffer to actual string length to save memory
    size_t actual_len = strlen(buffer);
    char* result = (char*)orion_malloc(actual_len + 1);
    if (!result) {
        fprintf(stderr, "Error: Failed to allocate memory for input result\n");
        exit(1);
    }
    strcpy(result, buffer);
    orion_free(buffer);
    
    return result;
}

// Input function with prompt - display prompt then read input
char* orion_input_prompt(const char* prompt) {
    if (prompt) {
        printf("%s", prompt);
        fflush(stdout);  // Ensure prompt is displayed before reading
    }
    
    return orion_input();
}

// Helper function to convert integer to string and append to buffer
// Returns pointer to the end of the buffer for chaining
char* sprintf_int(char* buffer, int64_t value) {
    if (!buffer) return buffer;
    
    // Find end of current string
    while (*buffer) buffer++;
    
    // Convert integer to string and append
    sprintf(buffer, "%ld", value);
    
    // Return pointer to new end of string
    while (*buffer) buffer++;
    return buffer;
}

// Simple string concatenation function
// Appends src to the end of dest and returns pointer to new end
char* strcat_simple(char* dest, const char* src) {
    if (!dest || !src) return dest;
    
    // Find end of dest string
    while (*dest) dest++;
    
    // Copy src to end of dest
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    
    return dest;
}

// Convert integer to string (returns dynamically allocated string)
char* int_to_string(int64_t value) {
    char* buffer = (char*)orion_malloc(32);  // Enough for 64-bit int
    if (!buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for int_to_string\n");
        exit(1);
    }
    sprintf(buffer, "%ld", value);
    return buffer;
}

// Convert float to string (returns dynamically allocated string)
char* float_to_string(double value) {
    char* buffer = (char*)orion_malloc(64);  // Enough for double precision
    if (!buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for float_to_string\n");
        exit(1);
    }
    sprintf(buffer, "%.2f", value);
    return buffer;
}

// Convert boolean to string (returns dynamically allocated string)
char* bool_to_string(int64_t value) {
    char* buffer = (char*)orion_malloc(8);  // "True" or "False"
    if (!buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for bool_to_string\n");
        exit(1);
    }
    strcpy(buffer, value ? "True" : "False");
    return buffer;
}

// Copy string (for consistency with other conversion functions)
char* string_to_string(const char* value) {
    if (!value) {
        char* buffer = (char*)orion_malloc(1);
        if (buffer) buffer[0] = '\0';
        return buffer;
    }
    
    size_t len = strlen(value);
    char* buffer = (char*)orion_malloc(len + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for string_to_string\n");
        exit(1);
    }
    strcpy(buffer, value);
    return buffer;
}

// String concatenation for interpolated strings
// Takes an array of string pointers and concatenates them
char* string_concat_parts(char** parts, int count) {
    if (!parts || count <= 0) {
        char* empty = (char*)orion_malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    
    // Calculate total length needed
    size_t total_len = 0;
    for (int i = 0; i < count; i++) {
        if (parts[i]) {
            total_len += strlen(parts[i]);
        }
    }
    
    // Allocate result buffer
    char* result = (char*)orion_malloc(total_len + 1);
    if (!result) {
        fprintf(stderr, "Error: Failed to allocate memory for string_concat_parts\n");
        exit(1);
    }
    
    // Concatenate all parts
    result[0] = '\0';
    for (int i = 0; i < count; i++) {
        if (parts[i]) {
            strcat(result, parts[i]);
        }
    }
    
    return result;
}