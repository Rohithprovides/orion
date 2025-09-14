# Orion Programming Language

## Overview
This project is a web-based IDE for Orion, a pure compiled systems programming language. Orion aims to combine C's performance with Python's readability. The platform features a Flask web server, a native C++ compiler, and a responsive web frontend, enabling users to write, compile, and execute Orion code directly in the browser. The language emphasizes direct machine code generation, explicit type annotations, C-style control flow with Python-like readability, and manual memory management for performance-critical applications.

## User Preferences
Preferred communication style: Simple, everyday language.

## System Architecture
### Frontend Architecture
The frontend is a single-page application built with vanilla JavaScript and Bootstrap 5 for responsive design. It includes a custom code editor with Prism.js for syntax highlighting and Feather icons for UI elements. Real-time compilation requests are handled via JavaScript.

### Backend Architecture
The backend uses Flask with CORS support to serve the web interface and interact with the native C++ compiler. It provides RESTful endpoints for compilation and execution. The execution model involves direct machine code generation without interpretation.

### Compiler Architecture
The Orion compiler is written in C++ and follows a traditional pipeline:
- **Lexer**: Tokenizes source code.
- **Parser**: Builds an Abstract Syntax Tree (AST) using recursive descent.
- **Type Checker**: Validates types and semantics via a visitor pattern.
- **Code Generator**: Produces x86-64 assembly code from the AST.
- **Binary**: A standalone executable for compiling and running Orion programs.

### Language Design Philosophy
Orion is a pure compiled language designed for performance and readability:
- **Type System**: Explicit type annotations (int, float, string, bool).
- **Function Syntax**: Clear declarations with return type annotations.
- **Control Flow**: C-style structures (if/else, for, while) with Python-like readability.
- **Memory Management**: Manual memory management with safer syntax.
- **Compilation Model**: Direct to machine code.
- **Function Behavior**: Python-style function execution, where functions only run when called, supporting nested functions and explicit calls (including `main()`).
- **Variable Scoping**: Python-style scoping with function-scope isolation, local variables by default, and `global` keyword support.
- **`const` Keyword**: Supports immutable variables with compile-time validation and clear error messages.
- **Arithmetic Operators**: Full Python-style arithmetic operators (+, -, \*, /, %, //, \*\*) with proper precedence and x86-64 assembly generation.
- **Compound Assignment Operators**: Python-style compound assignment operators (+=, -=, \*=, /=, %=) implemented via desugaring at parse time.
- **Loop Implementation**: Comprehensive Python-style loop support including `while` and `for-in` loops, with `break`, `continue`, and `pass` statements, and proper handling of nested loops.

## External Dependencies
### Frontend Dependencies
- **Bootstrap 5.3.0**: CSS framework.
- **Prism.js 1.29.0**: Syntax highlighting library.
- **Feather Icons**: Lightweight icon library.

### Backend Dependencies
- **Flask**: Python web framework.
- **Flask-CORS**: Cross-Origin Resource Sharing extension.
- **C++ Compiler**: Native g++ for building the Orion compiler.
- **Gunicorn**: Production WSGI HTTP Server for Python web applications (for deployment).