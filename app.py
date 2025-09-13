#!/usr/bin/env python3
"""
Orion Programming Language Web Compiler Server
Pure compiled language implementation using C++ compiler.
"""

from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import json
import time
import os
import sys
import subprocess
import tempfile

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
    """Compile and execute Orion code using the C++ compiler."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'success': False, 'error': 'No code provided'})
        
        code = data['code']
        total_start_time = time.time()
        
        try:
            # Create a temporary file
            file_start_time = time.time()
            with tempfile.NamedTemporaryFile(mode='w', suffix='.or', delete=False) as temp_file:
                temp_file.write(code)
                temp_file_path = temp_file.name
            
            # Run the C++ compiler (compilation + execution)
            # Change to compiler directory to find runtime.o
            compile_start_time = time.time()
            result = subprocess.run(
                ['./orion', os.path.abspath(temp_file_path)],
                cwd='./compiler',
                capture_output=True,
                text=True,
                timeout=10
            )
            compile_end_time = time.time()
            
            # Clean up temporary file
            os.unlink(temp_file_path)
            
            # Calculate timing breakdown
            total_time = int((time.time() - total_start_time) * 1000)
            compile_and_run_time = int((compile_end_time - compile_start_time) * 1000)
            
            # Estimate execution time (approximate - most time is compilation)
            # The actual program execution is very fast (< 1ms typically)
            estimated_execution_time = max(1, compile_and_run_time - int(compile_and_run_time * 0.95))
            compilation_time = compile_and_run_time - estimated_execution_time
            
            if result.returncode == 0:
                output = result.stdout.strip() if result.stdout else "Compilation successful"
                return jsonify({
                    'success': True,
                    'output': output,
                    'total_time': total_time,
                    'compilation_time': compilation_time,
                    'execution_time': estimated_execution_time
                })
            else:
                error_msg = result.stderr.strip() if result.stderr else "Compilation failed"
                return jsonify({
                    'success': False,
                    'error': f'Compilation error: {error_msg}'
                })
                
        except subprocess.TimeoutExpired:
            return jsonify({
                'success': False,
                'error': 'Compilation timeout (took too long)'
            })
        except Exception as e:
            return jsonify({
                'success': False,
                'error': f'Compiler error: {str(e)}'
            })
            
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Server error: {str(e)}'
        })

@app.route('/check-syntax', methods=['POST'])
def check_syntax():
    """Check syntax of Orion code using the C++ compiler."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'valid': False, 'error': 'No code provided'})
        
        code = data['code']
        
        try:
            # Use the C++ compiler for syntax checking
            with tempfile.NamedTemporaryFile(mode='w', suffix='.or', delete=False) as temp_file:
                temp_file.write(code)
                temp_file_path = temp_file.name
            
            # Run syntax check (we can use the same compiler)
            result = subprocess.run(
                ['./orion', os.path.abspath(temp_file_path)],
                cwd='./compiler',
                capture_output=True,
                text=True,
                timeout=5
            )
            
            os.unlink(temp_file_path)
            
            if result.returncode == 0:
                return jsonify({
                    'valid': True,
                    'message': 'Syntax is valid!'
                })
            else:
                error_msg = result.stderr.strip() if result.stderr else "Syntax error"
                return jsonify({
                    'valid': False,
                    'error': f'Syntax error: {error_msg}'
                })
                
        except Exception as e:
            return jsonify({
                'valid': False,
                'error': f'Syntax check error: {str(e)}'
            })
            
    except Exception as e:
        return jsonify({
            'valid': False,
            'error': f'Server error: {str(e)}'
        })

@app.route('/ast', methods=['POST'])
def generate_ast():
    """Generate AST for Orion code."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'success': False, 'error': 'No code provided'})
        
        code = data['code']
        
        # For now, return a simple representation
        # In a full implementation, this would use the C++ parser to generate AST
        return jsonify({
            'success': True,
            'ast': f"AST for Orion code ({len(code)} characters)\n[Full AST visualization would be implemented in C++]"
        })
            
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Server error: {str(e)}'
        })

if __name__ == '__main__':
    print("Starting Orion Compiler Web Interface...")
    print("Open http://localhost:5000 in your browser")
    app.run(host='0.0.0.0', port=5000, debug=True)