#!/usr/bin/env python3
"""
Orion Programming Language Web Compiler Server
"""

from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import json
import time
import os
import sys
import subprocess
import tempfile
from orion_interpreter import execute_orion_code
from orion_lexer import tokenize_orion_code, TokenType
from orion_parser import parse_orion_code, ParseError
from orion_interpreter import RuntimeError as OrionRuntimeError

app = Flask(__name__)
CORS(app)

@app.route('/')
def index():
    """Serve the main HTML page."""
    return send_from_directory('.', 'index.html')

@app.route('/<path:filename>')
def serve_static(filename):
    """Serve static files."""
    return send_from_directory('.', filename)

@app.route('/compile', methods=['POST'])
def compile_code():
    """Compile and execute Orion code using the interpreter or C++ compiler."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'success': False, 'error': 'No code provided'})
        
        code = data['code']
        compiler_type = data.get('compiler', 'interpreter')  # 'interpreter' or 'native'
        
        start_time = time.time()
        
        if compiler_type == 'native':
            # Use the C++ compiler
            try:
                # Create a temporary file
                with tempfile.NamedTemporaryFile(mode='w', suffix='.or', delete=False) as temp_file:
                    temp_file.write(code)
                    temp_file_path = temp_file.name
                
                # Run the C++ compiler
                result = subprocess.run(
                    ['./compiler/orion', temp_file_path],
                    capture_output=True,
                    text=True,
                    timeout=10
                )
                
                # Clean up temporary file
                os.unlink(temp_file_path)
                
                execution_time = int((time.time() - start_time) * 1000)
                
                if result.returncode == 0:
                    output = result.stdout.strip() if result.stdout else "Native compilation successful"
                    return jsonify({
                        'success': True,
                        'output': output + "\n[Compiled with C++ native compiler]",
                        'execution_time': execution_time,
                        'compiler': 'native'
                    })
                else:
                    error_msg = result.stderr.strip() if result.stderr else "Compilation failed"
                    return jsonify({
                        'success': False,
                        'error': f'C++ Compiler error: {error_msg}',
                        'compiler': 'native'
                    })
                    
            except subprocess.TimeoutExpired:
                return jsonify({
                    'success': False,
                    'error': 'Compilation timeout (C++ compiler took too long)',
                    'compiler': 'native'
                })
            except Exception as e:
                return jsonify({
                    'success': False,
                    'error': f'C++ Compiler error: {str(e)}',
                    'compiler': 'native'
                })
        else:
            # Use the Python interpreter (default)
            try:
                result, output = execute_orion_code(code)
                execution_time = int((time.time() - start_time) * 1000)
                
                if not output:
                    output = "Program executed successfully (no output)"
                
                return jsonify({
                    'success': True,
                    'output': output.strip() + "\n[Executed with Python interpreter]",
                    'execution_time': execution_time,
                    'return_value': str(result) if result is not None else None,
                    'compiler': 'interpreter'
                })
                
            except ParseError as e:
                return jsonify({
                    'success': False,
                    'error': f'Parse error: {e.message}' + (f' at line {e.token.line}, column {e.token.column}' if e.token else ''),
                    'compiler': 'interpreter'
                })
            except OrionRuntimeError as e:
                return jsonify({
                    'success': False,
                    'error': f'Runtime error: {e.message}',
                    'compiler': 'interpreter'
                })
            except Exception as e:
                return jsonify({
                    'success': False,
                    'error': f'Execution error: {str(e)}',
                    'compiler': 'interpreter'
                })
            
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Server error: {str(e)}'
        })

@app.route('/check-syntax', methods=['POST'])
def check_syntax():
    """Check syntax of Orion code using the real lexer."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'valid': False, 'error': 'No code provided'})
        
        code = data['code']
        
        try:
            # Use the real Orion lexer
            tokens = tokenize_orion_code(code)
            
            # Convert tokens to a format suitable for JSON
            token_list = []
            for token in tokens:
                if token.type not in [TokenType.NEWLINE, TokenType.COMMENT, TokenType.EOF]:
                    token_list.append({
                        'type': token.type.value,
                        'value': token.value,
                        'line': token.line,
                        'column': token.column
                    })
            
            # Try to parse to check for syntax errors
            try:
                ast = parse_orion_code(code)
                return jsonify({
                    'valid': True,
                    'tokens': token_list[:20],  # Limit to first 20 tokens
                    'message': f'Syntax is valid. Found {len(token_list)} tokens.'
                })
            except ParseError as e:
                return jsonify({
                    'valid': False,
                    'error': f'Parse error: {e.message}' + (f' at line {e.token.line}, column {e.token.column}' if e.token else ''),
                    'tokens': token_list[:20]
                })
                
        except Exception as e:
            return jsonify({
                'valid': False,
                'error': f'Lexical error: {str(e)}'
            })
        
    except Exception as e:
        return jsonify({
            'valid': False,
            'error': f'Syntax check error: {str(e)}'
        })

@app.route('/ast', methods=['POST'])
def generate_ast():
    """Generate AST for Orion code using the real parser."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'success': False, 'error': 'No code provided'})
        
        code = data['code']
        
        try:
            # Use the real Orion parser
            ast = parse_orion_code(code)
            
            # Convert AST to a readable string representation
            ast_string = _ast_to_string(ast)
            
            return jsonify({
                'success': True,
                'ast': ast_string
            })
            
        except ParseError as e:
            return jsonify({
                'success': False,
                'error': f'Parse error: {e.message}' + (f' at line {e.token.line}, column {e.token.column}' if e.token else '')
            })
        except Exception as e:
            return jsonify({
                'success': False,
                'error': f'AST generation error: {str(e)}'
            })
        
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Server error: {str(e)}'
        })

def _ast_to_string(node, indent=0):
    """Convert AST node to readable string representation"""
    prefix = "  " * indent
    
    if hasattr(node, '__class__'):
        result = f"{prefix}{node.__class__.__name__}"
        
        # Add specific details for different node types
        if hasattr(node, 'name'):
            result += f" (name: {node.name})"
        elif hasattr(node, 'value'):
            result += f" (value: {repr(node.value)})"
        elif hasattr(node, 'operator'):
            result += f" (op: {node.operator})"
        
        result += "\n"
        
        # Recursively add child nodes
        if hasattr(node, 'statements') and node.statements:
            for stmt in node.statements:
                result += _ast_to_string(stmt, indent + 1)
        elif hasattr(node, 'body') and node.body:
            result += _ast_to_string(node.body, indent + 1)
        elif hasattr(node, 'left') and hasattr(node, 'right'):
            result += _ast_to_string(node.left, indent + 1)
            result += _ast_to_string(node.right, indent + 1)
        elif hasattr(node, 'expression') and node.expression:
            result += _ast_to_string(node.expression, indent + 1)
        elif hasattr(node, 'arguments') and node.arguments:
            for arg in node.arguments:
                result += _ast_to_string(arg, indent + 1)
        
        return result
    else:
        return f"{prefix}{repr(node)}\n"

if __name__ == '__main__':
    print("Starting Orion Compiler Web Interface...")
    print("Open http://localhost:5000 in your browser")
    app.run(host='0.0.0.0', port=5000, debug=True)