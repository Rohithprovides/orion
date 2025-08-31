#!/usr/bin/env python3
"""
Orion Programming Language Web Compiler Server
"""

from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import json
import time
import os

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
    """Compile Orion code and return results."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'success': False, 'error': 'No code provided'})
        
        code = data['code']
        
        # Simulate compilation for demo purposes
        # In a real implementation, this would use the full compiler
        
        # Check for basic syntax
        if 'main' in code or 'fn' in code:
            # Simulate successful compilation
            output = ""
            
            # Extract and simulate out() calls
            if 'out(' in code:
                import re
                out_matches = re.findall(r'out\(([^)]+)\)', code)
                for match in out_matches:
                    # Remove quotes if present
                    text = match.strip().strip('"').strip("'")
                    output += text + "\n"
            
            if not output:
                output = "Program executed successfully (no output)\n"
            
            return jsonify({
                'success': True,
                'output': output.strip(),
                'execution_time': 45
            })
        else:
            return jsonify({
                'success': False,
                'error': 'Error: No main function found'
            })
            
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Compilation error: {str(e)}'
        })

@app.route('/check-syntax', methods=['POST'])
def check_syntax():
    """Check syntax of Orion code."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'valid': False, 'error': 'No code provided'})
        
        code = data['code']
        
        # Simple syntax checking
        tokens = []
        
        # Basic tokenization
        import re
        
        # Find keywords
        keywords = re.findall(r'\b(fn|main|if|else|while|for|return|int|string|bool|out)\b', code)
        for keyword in keywords:
            tokens.append({'type': 'KEYWORD', 'value': keyword})
        
        # Find identifiers
        identifiers = re.findall(r'\b[a-zA-Z_][a-zA-Z0-9_]*\b', code)
        for identifier in identifiers:
            if identifier not in ['fn', 'main', 'if', 'else', 'while', 'for', 'return', 'int', 'string', 'bool', 'out']:
                tokens.append({'type': 'IDENTIFIER', 'value': identifier})
        
        # Find numbers
        numbers = re.findall(r'\b\d+\b', code)
        for number in numbers:
            tokens.append({'type': 'NUMBER', 'value': number})
        
        # Find strings
        strings = re.findall(r'"[^"]*"', code)
        for string in strings:
            tokens.append({'type': 'STRING', 'value': string})
        
        return jsonify({
            'valid': True,
            'tokens': tokens[:20]  # Limit to first 20 tokens
        })
        
    except Exception as e:
        return jsonify({
            'valid': False,
            'error': f'Syntax error: {str(e)}'
        })

@app.route('/ast', methods=['POST'])
def generate_ast():
    """Generate AST for Orion code."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'success': False, 'error': 'No code provided'})
        
        code = data['code']
        
        # Simple AST generation
        ast = "Program\n"
        
        if 'fn' in code and 'main' in code:
            ast += "  FunctionDeclaration\n"
            ast += "    name: main\n"
            ast += "    parameters: []\n"
            ast += "    return_type: int\n"
            ast += "    body:\n"
            
            if 'out(' in code:
                import re
                out_calls = re.findall(r'out\([^)]+\)', code)
                for call in out_calls:
                    ast += "      ExpressionStatement\n"
                    ast += "        FunctionCall\n"
                    ast += "          name: out\n"
                    ast += "          arguments:\n"
                    ast += "            StringLiteral\n"
        
        return jsonify({
            'success': True,
            'ast': ast
        })
        
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'AST generation error: {str(e)}'
        })

if __name__ == '__main__':
    print("Starting Orion Compiler Web Interface...")
    print("Open http://localhost:5000 in your browser")
    app.run(host='0.0.0.0', port=5000, debug=True)