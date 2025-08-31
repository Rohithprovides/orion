#!/usr/bin/env python3
"""
Orion Programming Language Web Compiler Server
Provides HTTP endpoints for compiling Orion code through a web interface.
"""

import json
import time
import re
import traceback
from typing import Dict, List, Any, Optional, Tuple
from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import os

app = Flask(__name__)
CORS(app)

class OrionToken:
    """Represents a token in Orion source code."""
    def __init__(self, token_type: str, value: str, line: int = 1, column: int = 1):
        self.type = token_type
        self.value = value
        self.line = line
        self.column = column
    
    def to_dict(self):
        return {
            'type': self.type,
            'value': self.value,
            'line': self.line,
            'column': self.column
        }

class OrionLexer:
    """Lexical analyzer for Orion programming language."""
    
    KEYWORDS = {
        'if', 'elif', 'else', 'while', 'for', 'return', 'struct', 'enum',
        'import', 'true', 'false', 'int', 'int64', 'float', 'float64', 
        'string', 'bool', 'void', 'main'
    }
    
    def __init__(self, source: str):
        self.source = source
        self.pos = 0
        self.line = 1
        self.column = 1
        self.tokens = []
    
    def error(self, message: str) -> Exception:
        return SyntaxError(f"Line {self.line}, Column {self.column}: {message}")
    
    def peek(self, offset: int = 0) -> str:
        pos = self.pos + offset
        if pos >= len(self.source):
            return '\0'
        return self.source[pos]
    
    def advance(self) -> str:
        if self.pos >= len(self.source):
            return '\0'
        char = self.source[self.pos]
        self.pos += 1
        if char == '\n':
            self.line += 1
            self.column = 1
        else:
            self.column += 1
        return char
    
    def skip_whitespace(self):
        while self.peek() in ' \t\r':
            self.advance()
    
    def skip_comment(self):
        if self.peek() == '/' and self.peek(1) == '/':
            # Single line comment
            while self.peek() != '\n' and self.peek() != '\0':
                self.advance()
        elif self.peek() == '/' and self.peek(1) == '*':
            # Multi-line comment
            self.advance()  # skip '/'
            self.advance()  # skip '*'
            while True:
                if self.peek() == '\0':
                    raise self.error("Unterminated comment")
                if self.peek() == '*' and self.peek(1) == '/':
                    self.advance()  # skip '*'
                    self.advance()  # skip '/'
                    break
                self.advance()
    
    def read_string(self) -> str:
        quote_char = self.advance()  # Skip opening quote
        value = ""
        
        while self.peek() != quote_char and self.peek() != '\0':
            char = self.peek()
            if char == '\\':
                self.advance()  # Skip backslash
                escaped = self.advance()
                if escaped == 'n':
                    value += '\n'
                elif escaped == 't':
                    value += '\t'
                elif escaped == 'r':
                    value += '\r'
                elif escaped == '\\':
                    value += '\\'
                elif escaped == '"':
                    value += '"'
                elif escaped == "'":
                    value += "'"
                else:
                    value += escaped
            else:
                value += self.advance()
        
        if self.peek() == '\0':
            raise self.error("Unterminated string")
        
        self.advance()  # Skip closing quote
        return value
    
    def read_number(self) -> Tuple[str, str]:
        value = ""
        token_type = "INTEGER"
        
        while self.peek().isdigit():
            value += self.advance()
        
        if self.peek() == '.' and self.peek(1).isdigit():
            token_type = "FLOAT"
            value += self.advance()  # decimal point
            while self.peek().isdigit():
                value += self.advance()
        
        return value, token_type
    
    def read_identifier(self) -> Tuple[str, str]:
        value = ""
        
        while self.peek().isalnum() or self.peek() == '_':
            value += self.advance()
        
        token_type = "KEYWORD" if value in self.KEYWORDS else "IDENTIFIER"
        return value, token_type
    
    def tokenize(self) -> List[OrionToken]:
        self.tokens = []
        
        while self.pos < len(self.source):
            self.skip_whitespace()
            
            if self.pos >= len(self.source):
                break
            
            # Comments
            if self.peek() == '/' and self.peek(1) in ['/', '*']:
                self.skip_comment()
                continue
            
            line, column = self.line, self.column
            char = self.peek()
            
            # Newlines
            if char == '\n':
                self.tokens.append(OrionToken("NEWLINE", "\\n", line, column))
                self.advance()
                continue
            
            # Strings
            if char in ['"', "'"]:
                value = self.read_string()
                self.tokens.append(OrionToken("STRING", value, line, column))
                continue
            
            # Numbers
            if char.isdigit():
                value, token_type = self.read_number()
                self.tokens.append(OrionToken(token_type, value, line, column))
                continue
            
            # Identifiers and keywords
            if char.isalpha() or char == '_':
                value, token_type = self.read_identifier()
                self.tokens.append(OrionToken(token_type, value, line, column))
                continue
            
            # Two-character operators
            two_char = char + self.peek(1)
            if two_char in ['==', '!=', '<=', '>=', '&&', '||', '++', '--', '+=', '-=', '->', '=>']:
                self.tokens.append(OrionToken("OPERATOR", two_char, line, column))
                self.advance()
                self.advance()
                continue
            
            # Single-character tokens
            single_char_tokens = {
                '+': 'OPERATOR', '-': 'OPERATOR', '*': 'OPERATOR', '/': 'OPERATOR', '%': 'OPERATOR',
                '=': 'OPERATOR', '<': 'OPERATOR', '>': 'OPERATOR', '!': 'OPERATOR',
                '(': 'LPAREN', ')': 'RPAREN', '{': 'LBRACE', '}': 'RBRACE',
                '[': 'LBRACKET', ']': 'RBRACKET', ';': 'SEMICOLON', ',': 'COMMA', '.': 'DOT'
            }
            
            if char in single_char_tokens:
                self.tokens.append(OrionToken(single_char_tokens[char], char, line, column))
                self.advance()
                continue
            
            # Unknown character
            raise self.error(f"Unexpected character: '{char}'")
        
        self.tokens.append(OrionToken("EOF", "", self.line, self.column))
        return self.tokens

class OrionASTNode:
    """Base class for AST nodes."""
    def __init__(self, node_type: str):
        self.type = node_type
        self.children = []
    
    def add_child(self, child):
        self.children.append(child)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            'type': self.type,
            'children': [child.to_dict() if hasattr(child, 'to_dict') else str(child) for child in self.children]
        }
    
    def __str__(self) -> str:
        return self._format_tree()
    
    def _format_tree(self, indent: int = 0) -> str:
        result = "  " * indent + self.type + "\n"
        for child in self.children:
            if isinstance(child, OrionASTNode):
                result += child._format_tree(indent + 1)
            else:
                result += "  " * (indent + 1) + str(child) + "\n"
        return result

class OrionParser:
    """Parser for Orion programming language."""
    
    def __init__(self, tokens: List[OrionToken]):
        self.tokens = tokens
        self.pos = 0
    
    def error(self, message: str) -> Exception:
        if self.pos < len(self.tokens):
            token = self.tokens[self.pos]
            return SyntaxError(f"Line {token.line}, Column {token.column}: {message}")
        return SyntaxError(f"End of file: {message}")
    
    def peek(self, offset: int = 0) -> Optional[OrionToken]:
        pos = self.pos + offset
        if pos >= len(self.tokens):
            return None
        return self.tokens[pos]
    
    def advance(self) -> Optional[OrionToken]:
        if self.pos >= len(self.tokens):
            return None
        token = self.tokens[self.pos]
        self.pos += 1
        return token
    
    def expect(self, token_type: str, value: str = None) -> OrionToken:
        token = self.advance()
        if not token or token.type != token_type:
            raise self.error(f"Expected {token_type}, got {token.type if token else 'EOF'}")
        if value and token.value != value:
            raise self.error(f"Expected '{value}', got '{token.value}'")
        return token
    
    def match(self, token_type: str, value: str = None) -> bool:
        token = self.peek()
        if not token or token.type != token_type:
            return False
        if value and token.value != value:
            return False
        return True
    
    def skip_newlines(self):
        while self.match("NEWLINE"):
            self.advance()
    
    def parse(self) -> OrionASTNode:
        """Parse the token stream into an AST."""
        program = OrionASTNode("Program")
        
        while self.pos < len(self.tokens) and not self.match("EOF"):
            self.skip_newlines()
            if self.match("EOF"):
                break
            
            stmt = self.parse_statement()
            if stmt:
                program.add_child(stmt)
        
        return program
    
    def parse_statement(self) -> Optional[OrionASTNode]:
        """Parse a statement."""
        # Function declaration
        if self.match("IDENTIFIER"):
            # Look ahead for function pattern
            saved_pos = self.pos
            self.advance()  # skip identifier
            if self.match("LPAREN"):
                self.pos = saved_pos  # restore position
                return self.parse_function()
        
        # Variable declaration
        if self.match("IDENTIFIER") or self.match("KEYWORD"):
            return self.parse_variable_declaration()
        
        # Control flow
        if self.match("KEYWORD", "if"):
            return self.parse_if_statement()
        if self.match("KEYWORD", "while"):
            return self.parse_while_statement()
        if self.match("KEYWORD", "for"):
            return self.parse_for_statement()
        if self.match("KEYWORD", "return"):
            return self.parse_return_statement()
        
        # Block statement
        if self.match("LBRACE"):
            return self.parse_block()
        
        # Expression statement
        expr = self.parse_expression()
        self.skip_newlines()
        return expr
    
    def parse_function(self) -> OrionASTNode:
        """Parse a function declaration."""
        func = OrionASTNode("FunctionDeclaration")
        
        # Function name
        name = self.expect("IDENTIFIER")
        func.add_child(f"name: {name.value}")
        
        # Parameters
        self.expect("LPAREN")
        params = OrionASTNode("Parameters")
        
        while not self.match("RPAREN"):
            if self.match("IDENTIFIER"):
                param_name = self.advance()
                param_type = "int"  # default type
                if self.match("KEYWORD"):
                    param_type = self.advance().value
                params.add_child(f"{param_name.value}: {param_type}")
            
            if self.match("COMMA"):
                self.advance()
            elif not self.match("RPAREN"):
                raise self.error("Expected ',' or ')' in parameter list")
        
        self.expect("RPAREN")
        func.add_child(params)
        
        # Return type
        return_type = "void"
        if self.match("OPERATOR", "->"):
            self.advance()
            if self.match("KEYWORD"):
                return_type = self.advance().value
        func.add_child(f"return_type: {return_type}")
        
        # Body
        if self.match("OPERATOR", "=>"):
            # Single expression
            self.advance()
            expr = self.parse_expression()
            func.add_child(expr)
        else:
            # Block body
            body = self.parse_block()
            func.add_child(body)
        
        return func
    
    def parse_variable_declaration(self) -> OrionASTNode:
        """Parse a variable declaration."""
        var_decl = OrionASTNode("VariableDeclaration")
        
        first_token = self.advance()
        
        if first_token.type == "KEYWORD" and first_token.value in ["int", "float", "string", "bool"]:
            # Type first: int x = 5
            var_type = first_token.value
            name = self.expect("IDENTIFIER")
            var_decl.add_child(f"name: {name.value}")
            var_decl.add_child(f"type: {var_type}")
        else:
            # Name first: x = 5 or x int = 5
            name = first_token.value
            var_decl.add_child(f"name: {name}")
            
            if self.match("KEYWORD"):
                # x int = 5
                var_type = self.advance().value
                var_decl.add_child(f"type: {var_type}")
            else:
                # x = 5 (type inference)
                var_decl.add_child("type: inferred")
        
        # Assignment
        if self.match("OPERATOR", "="):
            self.advance()
            expr = self.parse_expression()
            var_decl.add_child(expr)
        
        self.skip_newlines()
        return var_decl
    
    def parse_if_statement(self) -> OrionASTNode:
        """Parse an if statement."""
        if_stmt = OrionASTNode("IfStatement")
        
        self.expect("KEYWORD", "if")
        condition = self.parse_expression()
        if_stmt.add_child(condition)
        
        then_stmt = self.parse_statement()
        if_stmt.add_child(then_stmt)
        
        if self.match("KEYWORD", "else"):
            self.advance()
            else_stmt = self.parse_statement()
            if_stmt.add_child(else_stmt)
        
        return if_stmt
    
    def parse_while_statement(self) -> OrionASTNode:
        """Parse a while statement."""
        while_stmt = OrionASTNode("WhileStatement")
        
        self.expect("KEYWORD", "while")
        condition = self.parse_expression()
        while_stmt.add_child(condition)
        
        body = self.parse_statement()
        while_stmt.add_child(body)
        
        return while_stmt
    
    def parse_for_statement(self) -> OrionASTNode:
        """Parse a for statement."""
        for_stmt = OrionASTNode("ForStatement")
        
        self.expect("KEYWORD", "for")
        
        # Simple for loop: for i = 0; i < 10; i++
        init = self.parse_statement()
        for_stmt.add_child(init)
        
        if self.match("SEMICOLON"):
            self.advance()
        
        condition = self.parse_expression()
        for_stmt.add_child(condition)
        
        if self.match("SEMICOLON"):
            self.advance()
        
        update = self.parse_expression()
        for_stmt.add_child(update)
        
        body = self.parse_statement()
        for_stmt.add_child(body)
        
        return for_stmt
    
    def parse_return_statement(self) -> OrionASTNode:
        """Parse a return statement."""
        return_stmt = OrionASTNode("ReturnStatement")
        
        self.expect("KEYWORD", "return")
        
        if not self.match("NEWLINE") and not self.match("SEMICOLON"):
            expr = self.parse_expression()
            return_stmt.add_child(expr)
        
        self.skip_newlines()
        return return_stmt
    
    def parse_block(self) -> OrionASTNode:
        """Parse a block statement."""
        block = OrionASTNode("BlockStatement")
        
        self.expect("LBRACE")
        self.skip_newlines()
        
        while not self.match("RBRACE") and not self.match("EOF"):
            stmt = self.parse_statement()
            if stmt:
                block.add_child(stmt)
            self.skip_newlines()
        
        self.expect("RBRACE")
        return block
    
    def parse_expression(self) -> OrionASTNode:
        """Parse an expression."""
        return self.parse_logical_or()
    
    def parse_logical_or(self) -> OrionASTNode:
        """Parse logical OR expression."""
        left = self.parse_logical_and()
        
        while self.match("OPERATOR", "||"):
            op = self.advance()
            right = self.parse_logical_and()
            
            bin_expr = OrionASTNode("BinaryExpression")
            bin_expr.add_child(left)
            bin_expr.add_child(f"operator: {op.value}")
            bin_expr.add_child(right)
            left = bin_expr
        
        return left
    
    def parse_logical_and(self) -> OrionASTNode:
        """Parse logical AND expression."""
        left = self.parse_equality()
        
        while self.match("OPERATOR", "&&"):
            op = self.advance()
            right = self.parse_equality()
            
            bin_expr = OrionASTNode("BinaryExpression")
            bin_expr.add_child(left)
            bin_expr.add_child(f"operator: {op.value}")
            bin_expr.add_child(right)
            left = bin_expr
        
        return left
    
    def parse_equality(self) -> OrionASTNode:
        """Parse equality expression."""
        left = self.parse_comparison()
        
        while self.match("OPERATOR") and self.peek().value in ["==", "!="]:
            op = self.advance()
            right = self.parse_comparison()
            
            bin_expr = OrionASTNode("BinaryExpression")
            bin_expr.add_child(left)
            bin_expr.add_child(f"operator: {op.value}")
            bin_expr.add_child(right)
            left = bin_expr
        
        return left
    
    def parse_comparison(self) -> OrionASTNode:
        """Parse comparison expression."""
        left = self.parse_term()
        
        while self.match("OPERATOR") and self.peek().value in ["<", "<=", ">", ">="]:
            op = self.advance()
            right = self.parse_term()
            
            bin_expr = OrionASTNode("BinaryExpression")
            bin_expr.add_child(left)
            bin_expr.add_child(f"operator: {op.value}")
            bin_expr.add_child(right)
            left = bin_expr
        
        return left
    
    def parse_term(self) -> OrionASTNode:
        """Parse term expression (+ -)."""
        left = self.parse_factor()
        
        while self.match("OPERATOR") and self.peek().value in ["+", "-"]:
            op = self.advance()
            right = self.parse_factor()
            
            bin_expr = OrionASTNode("BinaryExpression")
            bin_expr.add_child(left)
            bin_expr.add_child(f"operator: {op.value}")
            bin_expr.add_child(right)
            left = bin_expr
        
        return left
    
    def parse_factor(self) -> OrionASTNode:
        """Parse factor expression (* / %)."""
        left = self.parse_unary()
        
        while self.match("OPERATOR") and self.peek().value in ["*", "/", "%"]:
            op = self.advance()
            right = self.parse_unary()
            
            bin_expr = OrionASTNode("BinaryExpression")
            bin_expr.add_child(left)
            bin_expr.add_child(f"operator: {op.value}")
            bin_expr.add_child(right)
            left = bin_expr
        
        return left
    
    def parse_unary(self) -> OrionASTNode:
        """Parse unary expression."""
        if self.match("OPERATOR") and self.peek().value in ["-", "+", "!"]:
            op = self.advance()
            expr = self.parse_unary()
            
            unary_expr = OrionASTNode("UnaryExpression")
            unary_expr.add_child(f"operator: {op.value}")
            unary_expr.add_child(expr)
            return unary_expr
        
        return self.parse_primary()
    
    def parse_primary(self) -> OrionASTNode:
        """Parse primary expression."""
        # Literals
        if self.match("INTEGER"):
            token = self.advance()
            literal = OrionASTNode("IntegerLiteral")
            literal.add_child(f"value: {token.value}")
            return literal
        
        if self.match("FLOAT"):
            token = self.advance()
            literal = OrionASTNode("FloatLiteral")
            literal.add_child(f"value: {token.value}")
            return literal
        
        if self.match("STRING"):
            token = self.advance()
            literal = OrionASTNode("StringLiteral")
            literal.add_child(f"value: \"{token.value}\"")
            return literal
        
        if self.match("KEYWORD") and self.peek().value in ["true", "false"]:
            token = self.advance()
            literal = OrionASTNode("BooleanLiteral")
            literal.add_child(f"value: {token.value}")
            return literal
        
        # Identifier or function call
        if self.match("IDENTIFIER"):
            name = self.advance()
            
            # Function call
            if self.match("LPAREN"):
                func_call = OrionASTNode("FunctionCall")
                func_call.add_child(f"name: {name.value}")
                
                self.advance()  # consume '('
                args = OrionASTNode("Arguments")
                
                while not self.match("RPAREN"):
                    arg = self.parse_expression()
                    args.add_child(arg)
                    
                    if self.match("COMMA"):
                        self.advance()
                    elif not self.match("RPAREN"):
                        raise self.error("Expected ',' or ')' in argument list")
                
                self.expect("RPAREN")
                func_call.add_child(args)
                return func_call
            else:
                # Simple identifier
                identifier = OrionASTNode("Identifier")
                identifier.add_child(f"name: {name.value}")
                return identifier
        
        # Parenthesized expression
        if self.match("LPAREN"):
            self.advance()
            expr = self.parse_expression()
            self.expect("RPAREN")
            return expr
        
        raise self.error(f"Unexpected token: {self.peek().value if self.peek() else 'EOF'}")

class OrionInterpreter:
    """Simple interpreter for demonstrating Orion code execution."""
    
    def __init__(self):
        self.variables = {}
        self.functions = {}
        self.output = []
    
    def execute(self, ast: OrionASTNode) -> Dict[str, Any]:
        """Execute the AST and return results."""
        self.output = []
        self.variables = {}
        self.functions = {}
        
        try:
            start_time = time.time()
            self.visit(ast)
            execution_time = int((time.time() - start_time) * 1000)
            
            return {
                'success': True,
                'output': '\n'.join(self.output),
                'execution_time': execution_time
            }
        except Exception as e:
            return {
                'success': False,
                'error': str(e),
                'execution_time': 0
            }
    
    def visit(self, node: OrionASTNode):
        """Visit an AST node."""
        if node.type == "Program":
            for child in node.children:
                if isinstance(child, OrionASTNode):
                    self.visit(child)
        
        elif node.type == "FunctionDeclaration":
            self.visit_function_declaration(node)
        
        elif node.type == "FunctionCall":
            return self.visit_function_call(node)
        
        elif node.type == "VariableDeclaration":
            self.visit_variable_declaration(node)
        
        elif node.type == "IntegerLiteral":
            return self.get_literal_value(node, int)
        
        elif node.type == "FloatLiteral":
            return self.get_literal_value(node, float)
        
        elif node.type == "StringLiteral":
            return self.get_string_literal_value(node)
        
        elif node.type == "BooleanLiteral":
            return self.get_literal_value(node, lambda x: x == "true")
        
        elif node.type == "Identifier":
            return self.visit_identifier(node)
        
        elif node.type == "BinaryExpression":
            return self.visit_binary_expression(node)
        
        elif node.type == "BlockStatement":
            for child in node.children:
                if isinstance(child, OrionASTNode):
                    result = self.visit(child)
                    if node.type == "ReturnStatement":
                        return result
        
        elif node.type == "ReturnStatement":
            if node.children:
                return self.visit(node.children[0])
            return None
        
        elif node.type == "ExpressionStatement":
            if node.children:
                return self.visit(node.children[0])
    
    def visit_function_declaration(self, node: OrionASTNode):
        """Visit a function declaration."""
        name = None
        for child in node.children:
            if isinstance(child, str) and child.startswith("name:"):
                name = child.split(": ", 1)[1]
                break
        
        if name:
            self.functions[name] = node
    
    def visit_function_call(self, node: OrionASTNode):
        """Visit a function call."""
        name = None
        args = []
        
        for child in node.children:
            if isinstance(child, str) and child.startswith("name:"):
                name = child.split(": ", 1)[1]
            elif isinstance(child, OrionASTNode) and child.type == "Arguments":
                for arg in child.children:
                    if isinstance(arg, OrionASTNode):
                        args.append(self.visit(arg))
        
        if name == "print":
            # Built-in print function
            if args:
                self.output.append(str(args[0]))
            return None
        elif name == "str":
            # Built-in string conversion
            if args:
                return str(args[0])
            return ""
        elif name in self.functions:
            # User-defined function
            func_node = self.functions[name]
            # Simple execution - just visit the function body
            for child in func_node.children:
                if isinstance(child, OrionASTNode) and child.type in ["BlockStatement", "ReturnStatement"]:
                    return self.visit(child)
        
        return None
    
    def visit_variable_declaration(self, node: OrionASTNode):
        """Visit a variable declaration."""
        name = None
        value = None
        
        for child in node.children:
            if isinstance(child, str) and child.startswith("name:"):
                name = child.split(": ", 1)[1]
            elif isinstance(child, OrionASTNode):
                value = self.visit(child)
        
        if name is not None:
            self.variables[name] = value if value is not None else 0
    
    def visit_identifier(self, node: OrionASTNode):
        """Visit an identifier."""
        for child in node.children:
            if isinstance(child, str) and child.startswith("name:"):
                name = child.split(": ", 1)[1]
                return self.variables.get(name, 0)
        return 0
    
    def visit_binary_expression(self, node: OrionASTNode):
        """Visit a binary expression."""
        left = None
        right = None
        operator = None
        
        for i, child in enumerate(node.children):
            if isinstance(child, OrionASTNode):
                if left is None:
                    left = self.visit(child)
                else:
                    right = self.visit(child)
            elif isinstance(child, str) and child.startswith("operator:"):
                operator = child.split(": ", 1)[1]
        
        if operator == "+":
            return left + right
        elif operator == "-":
            return left - right
        elif operator == "*":
            return left * right
        elif operator == "/":
            return left / right if right != 0 else 0
        elif operator == "%":
            return left % right if right != 0 else 0
        elif operator == "==":
            return left == right
        elif operator == "!=":
            return left != right
        elif operator == "<":
            return left < right
        elif operator == "<=":
            return left <= right
        elif operator == ">":
            return left > right
        elif operator == ">=":
            return left >= right
        
        return 0
    
    def get_literal_value(self, node: OrionASTNode, converter):
        """Get the value of a literal node."""
        for child in node.children:
            if isinstance(child, str) and child.startswith("value:"):
                value_str = child.split(": ", 1)[1]
                return converter(value_str)
        return converter("0")
    
    def get_string_literal_value(self, node: OrionASTNode):
        """Get the value of a string literal node."""
        for child in node.children:
            if isinstance(child, str) and child.startswith("value:"):
                value_str = child.split(": ", 1)[1]
                # Remove surrounding quotes
                if value_str.startswith('"') and value_str.endswith('"'):
                    return value_str[1:-1]
                return value_str
        return ""

class OrionCompiler:
    """Main compiler class that orchestrates lexing, parsing, and execution."""
    
    def compile_and_run(self, source: str) -> Dict[str, Any]:
        """Compile and run Orion source code."""
        try:
            # Lexical analysis
            lexer = OrionLexer(source)
            tokens = lexer.tokenize()
            
            # Parsing
            parser = OrionParser(tokens)
            ast = parser.parse()
            
            # Type checking would go here in a full implementation
            
            # Execution (interpretation for demo)
            interpreter = OrionInterpreter()
            result = interpreter.execute(ast)
            
            return result
            
        except Exception as e:
            return {
                'success': False,
                'error': str(e),
                'execution_time': 0
            }
    
    def check_syntax(self, source: str) -> Dict[str, Any]:
        """Check syntax and return tokens."""
        try:
            lexer = OrionLexer(source)
            tokens = lexer.tokenize()
            
            # Filter out EOF token for display
            display_tokens = [token for token in tokens if token.type != "EOF"]
            
            return {
                'valid': True,
                'tokens': [token.to_dict() for token in display_tokens]
            }
            
        except Exception as e:
            return {
                'valid': False,
                'error': str(e)
            }
    
    def generate_ast(self, source: str) -> Dict[str, Any]:
        """Generate and return AST."""
        try:
            lexer = OrionLexer(source)
            tokens = lexer.tokenize()
            
            parser = OrionParser(tokens)
            ast = parser.parse()
            
            return {
                'success': True,
                'ast': str(ast)
            }
            
        except Exception as e:
            return {
                'success': False,
                'error': str(e)
            }

# Global compiler instance
compiler = OrionCompiler()

# Web interface routes
@app.route('/')
def index():
    """Serve the main HTML page."""
    return send_from_directory('.', 'index.html')

@app.route('/<path:filename>')
def static_files(filename):
    """Serve static files (CSS, JS, etc.)."""
    return send_from_directory('.', filename)

@app.route('/compile', methods=['POST'])
def compile_code():
    """Compile and run Orion code."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'success': False, 'error': 'No code provided'}), 400
        
        source_code = data['code']
        result = compiler.compile_and_run(source_code)
        
        return jsonify(result)
        
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Server error: {str(e)}',
            'execution_time': 0
        }), 500

@app.route('/check-syntax', methods=['POST'])
def check_syntax():
    """Check syntax of Orion code."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'valid': False, 'error': 'No code provided'}), 400
        
        source_code = data['code']
        result = compiler.check_syntax(source_code)
        
        return jsonify(result)
        
    except Exception as e:
        return jsonify({
            'valid': False,
            'error': f'Server error: {str(e)}'
        }), 500

@app.route('/ast', methods=['POST'])
def generate_ast():
    """Generate AST for Orion code."""
    try:
        data = request.get_json()
        if not data or 'code' not in data:
            return jsonify({'success': False, 'error': 'No code provided'}), 400
        
        source_code = data['code']
        result = compiler.generate_ast(source_code)
        
        return jsonify(result)
        
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Server error: {str(e)}'
        }), 500

@app.errorhandler(404)
def not_found(error):
    """Handle 404 errors."""
    return jsonify({'error': 'Endpoint not found'}), 404

@app.errorhandler(500)
def internal_error(error):
    """Handle 500 errors."""
    return jsonify({'error': 'Internal server error'}), 500

if __name__ == '__main__':
    print("Starting Orion Programming Language Compiler Server...")
    print("Access the web interface at: http://localhost:5000")
    
    # Run the Flask development server
    app.run(host='0.0.0.0', port=5000, debug=True)
