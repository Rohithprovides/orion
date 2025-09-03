# Orion Programming Language

## Overview

This is a web-based IDE for the Orion programming language, a pure compiled systems programming language designed to combine C's performance with Python's readability. The project consists of a Flask-based web server that interfaces with a native C++ compiler backend, providing a responsive web frontend for writing, compiling, and executing Orion code directly in the browser.

## User Preferences

Preferred communication style: Simple, everyday language.

## System Architecture

### Frontend Architecture
The frontend is built as a single-page application using vanilla JavaScript with Bootstrap for styling and responsive design:

- **UI Framework**: Bootstrap 5 for responsive layout and components
- **Code Editor**: Custom textarea-based editor with syntax highlighting support via Prism.js
- **Icon System**: Feather icons for consistent iconography
- **Real-time Compilation**: JavaScript handles communication with the native C++ compiler via HTTP requests

The frontend is organized into separate concerns: HTML structure (index.html), styling (style.css), and interactive behavior (script.js). This separation allows for maintainable code and easy modification of individual components.

### Backend Architecture
The backend uses Flask as a web server that interfaces with a native C++ compiler:

- **Web Framework**: Flask with CORS support for cross-origin requests
- **Compilation**: Native C++ compiler with complete lexer, parser, AST, type checker, and code generator
- **API Design**: RESTful endpoints for code compilation and execution
- **Execution Model**: Direct machine code generation and execution (no interpretation)

### Compiler Architecture
The Orion compiler is built in C++ with a traditional compiler pipeline:

- **Lexer**: Tokenizes Orion source code into structured tokens
- **Parser**: Builds Abstract Syntax Tree (AST) from tokens using recursive descent parsing
- **Type Checker**: Validates types and semantics using visitor pattern
- **Code Generator**: Produces x86-64 assembly code from validated AST
- **Binary**: Standalone executable that can compile and run Orion programs

### Language Design Philosophy
Orion is designed as a pure compiled language with specific syntax choices that balance performance and readability:

- **Type System**: Explicit type annotations with support for common types (int, float, string, bool)
- **Function Syntax**: Clear function declarations with return type annotations
- **Control Flow**: Familiar C-style control structures (if/else, for, while) with Python-like readability
- **Memory Management**: Manual memory management like C with safer syntax patterns
- **Compilation Model**: Direct to machine code compilation (no interpretation or virtual machine)

## External Dependencies

### Frontend Dependencies
- **Bootstrap 5.3.0**: CSS framework for responsive UI components and layout
- **Prism.js 1.29.0**: Syntax highlighting library for code display
- **Feather Icons**: Lightweight icon library for UI elements

### Backend Dependencies
- **Flask**: Python web framework for HTTP server and routing (web interface only)
- **Flask-CORS**: Cross-Origin Resource Sharing support for browser compatibility
- **C++ Compiler**: Native g++ for building the Orion compiler

## Recent Updates (September 2025)

### Python-Style Function Behavior
The Orion compiler now implements Python-style function execution and nested function support:

- **Functions Only Run When Called**: Functions are defined but do not execute automatically - they only run when explicitly called
- **Nested Function Support**: Functions can be defined inside other functions and called properly
- **Explicit Function Calling**: All functions, including `main()`, must be called explicitly to execute
- **Proper Function Scoping**: Nested functions have access to their containing function's scope
- **No Automatic Execution**: The compiler no longer automatically executes the `main()` function or any other functions

### Variable Scoping System
The Orion compiler implements proper Python-style variable scoping with complete scope isolation:

- **Function Scope Isolation**: Variables defined inside functions only exist within that function's scope
- **Local Variables by Default**: Variable assignments inside functions create local variables that are inaccessible outside the function
- **Global Keyword Support**: Using `global varname` allows functions to create or modify global variables
- **Proper Error Handling**: Accessing variables outside their defining scope throws "Undefined variable" errors
- **Variable Lifetime Management**: Local variables are properly destroyed when function scope exits
- **Nested Function Support**: Functions defined inside other functions have proper scope chain resolution

### Deployment Configuration
The project is fully configured for Replit deployment with:
- **Autoscale deployment** target for efficient resource usage
- **Gunicorn production server** configuration
- **Port 5000 binding** for web interface compatibility
- **Environment variable management** for session secrets

The system is designed with a clear separation between the web interface (Python/Flask) and the language implementation (C++). The Orion language itself is purely compiled with no runtime interpretation, making it suitable for systems programming and performance-critical applications.