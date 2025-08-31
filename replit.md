# Orion Programming Language Compiler

## Overview

This is a web-based compiler and interactive development environment for the Orion programming language, a systems programming language designed to combine C's performance with Python's readability. The project consists of a Flask-based backend compiler/interpreter and a responsive web frontend that allows users to write, compile, and execute Orion code directly in the browser.

## User Preferences

Preferred communication style: Simple, everyday language.

## System Architecture

### Frontend Architecture
The frontend is built as a single-page application using vanilla JavaScript with Bootstrap for styling and responsive design. The architecture follows a simple client-server model:

- **UI Framework**: Bootstrap 5 for responsive layout and components
- **Code Editor**: Custom textarea-based editor with syntax highlighting support via Prism.js
- **Icon System**: Feather icons for consistent iconography
- **Real-time Compilation**: JavaScript handles communication with the Python backend via HTTP requests

The frontend is organized into separate concerns: HTML structure (index.html), styling (style.css), and interactive behavior (script.js). This separation allows for maintainable code and easy modification of individual components.

### Backend Architecture
The backend uses Flask as the web framework with a custom-built lexer and compiler for the Orion language:

- **Web Framework**: Flask with CORS support for cross-origin requests
- **Language Processing**: Custom lexer (OrionLexer) and token system (OrionToken) for parsing Orion source code
- **API Design**: RESTful endpoints for code compilation and execution
- **Error Handling**: Comprehensive error reporting with line/column information for debugging

The compiler architecture is designed as a traditional two-phase system with lexical analysis followed by parsing/interpretation. The lexer tokenizes the source code into structured tokens that can be processed by subsequent compilation phases.

### Language Design Philosophy
Orion is designed with specific syntax choices that balance performance and readability:

- **Type System**: Explicit type annotations with support for common types (int, float, string, bool)
- **Function Syntax**: Clear function declarations with return type annotations
- **Control Flow**: Familiar C-style control structures (if/else, for, while) with Python-like readability
- **Memory Management**: Designed for manual memory management like C but with safer syntax patterns

## External Dependencies

### Frontend Dependencies
- **Bootstrap 5.3.0**: CSS framework for responsive UI components and layout
- **Prism.js 1.29.0**: Syntax highlighting library for code display
- **Feather Icons**: Lightweight icon library for UI elements

### Backend Dependencies
- **Flask**: Python web framework for HTTP server and routing
- **Flask-CORS**: Cross-Origin Resource Sharing support for browser compatibility

The system is designed to be self-contained with minimal external dependencies, making it easy to deploy and maintain. All frontend dependencies are loaded via CDN for simplicity, while the backend uses standard Python libraries available through pip.