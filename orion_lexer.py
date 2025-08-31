"""
Orion Programming Language Lexer
Tokenizes Orion source code into a stream of tokens
"""

import re
from enum import Enum
from dataclasses import dataclass
from typing import List, Optional, Iterator

class TokenType(Enum):
    # Literals
    NUMBER = "NUMBER"
    STRING = "STRING"
    IDENTIFIER = "IDENTIFIER"
    
    # Keywords
    FN = "FN"
    MAIN = "MAIN"
    IF = "IF"
    ELSE = "ELSE"
    WHILE = "WHILE"
    FOR = "FOR"
    RETURN = "RETURN"
    OUT = "OUT"
    
    # Types
    INT = "INT"
    FLOAT = "FLOAT"
    STRING_TYPE = "STRING_TYPE"
    BOOL = "BOOL"
    
    # Operators
    ASSIGN = "ASSIGN"
    PLUS = "PLUS"
    MINUS = "MINUS"
    MULTIPLY = "MULTIPLY"
    DIVIDE = "DIVIDE"
    MODULO = "MODULO"
    
    # Comparison
    EQUAL = "EQUAL"
    NOT_EQUAL = "NOT_EQUAL"
    LESS_THAN = "LESS_THAN"
    GREATER_THAN = "GREATER_THAN"
    LESS_EQUAL = "LESS_EQUAL"
    GREATER_EQUAL = "GREATER_EQUAL"
    
    # Logical
    AND = "AND"
    OR = "OR"
    NOT = "NOT"
    
    # Delimiters
    LPAREN = "LPAREN"
    RPAREN = "RPAREN"
    LBRACE = "LBRACE"
    RBRACE = "RBRACE"
    LBRACKET = "LBRACKET"
    RBRACKET = "RBRACKET"
    SEMICOLON = "SEMICOLON"
    COMMA = "COMMA"
    DOT = "DOT"
    
    # Special
    NEWLINE = "NEWLINE"
    EOF = "EOF"
    COMMENT = "COMMENT"

@dataclass
class Token:
    type: TokenType
    value: str
    line: int
    column: int

class OrionLexer:
    def __init__(self, source: str):
        self.source = source
        self.position = 0
        self.line = 1
        self.column = 1
        self.tokens: List[Token] = []
        
        # Keywords mapping
        self.keywords = {
            'fn': TokenType.FN,
            'main': TokenType.MAIN,
            'if': TokenType.IF,
            'else': TokenType.ELSE,
            'while': TokenType.WHILE,
            'for': TokenType.FOR,
            'return': TokenType.RETURN,
            'out': TokenType.OUT,
            'int': TokenType.INT,
            'float': TokenType.FLOAT,
            'string': TokenType.STRING_TYPE,
            'bool': TokenType.BOOL,
            'and': TokenType.AND,
            'or': TokenType.OR,
            'not': TokenType.NOT,
        }
        
        # Operators mapping
        self.operators = {
            '=': TokenType.ASSIGN,
            '+': TokenType.PLUS,
            '-': TokenType.MINUS,
            '*': TokenType.MULTIPLY,
            '/': TokenType.DIVIDE,
            '%': TokenType.MODULO,
            '==': TokenType.EQUAL,
            '!=': TokenType.NOT_EQUAL,
            '<': TokenType.LESS_THAN,
            '>': TokenType.GREATER_THAN,
            '<=': TokenType.LESS_EQUAL,
            '>=': TokenType.GREATER_EQUAL,
        }
        
        # Single character tokens
        self.single_chars = {
            '(': TokenType.LPAREN,
            ')': TokenType.RPAREN,
            '{': TokenType.LBRACE,
            '}': TokenType.RBRACE,
            '[': TokenType.LBRACKET,
            ']': TokenType.RBRACKET,
            ';': TokenType.SEMICOLON,
            ',': TokenType.COMMA,
            '.': TokenType.DOT,
        }
    
    def current_char(self) -> Optional[str]:
        """Get the current character"""
        if self.position >= len(self.source):
            return None
        return self.source[self.position]
    
    def peek_char(self, offset: int = 1) -> Optional[str]:
        """Peek at the next character without consuming it"""
        pos = self.position + offset
        if pos >= len(self.source):
            return None
        return self.source[pos]
    
    def advance(self) -> Optional[str]:
        """Consume and return the current character"""
        char = self.current_char()
        if char is not None:
            self.position += 1
            if char == '\n':
                self.line += 1
                self.column = 1
            else:
                self.column += 1
        return char
    
    def skip_whitespace(self):
        """Skip whitespace characters except newlines"""
        while self.current_char() and self.current_char() in ' \t\r':
            self.advance()
    
    def read_string(self) -> str:
        """Read a string literal"""
        quote_char = self.advance()  # consume opening quote
        value = ""
        
        while self.current_char() and self.current_char() != quote_char:
            char = self.advance()
            if char == '\\':  # Handle escape sequences
                next_char = self.advance()
                if next_char == 'n':
                    value += '\n'
                elif next_char == 't':
                    value += '\t'
                elif next_char == 'r':
                    value += '\r'
                elif next_char == '\\':
                    value += '\\'
                elif next_char == quote_char:
                    value += quote_char
                else:
                    value += next_char
            else:
                value += char
        
        if self.current_char() == quote_char:
            self.advance()  # consume closing quote
        
        return value
    
    def read_number(self) -> str:
        """Read a number (integer or float)"""
        value = ""
        has_dot = False
        
        while self.current_char() and (self.current_char().isdigit() or self.current_char() == '.'):
            char = self.current_char()
            if char == '.':
                if has_dot:
                    break  # Second dot, stop here
                has_dot = True
            value += self.advance()
        
        return value
    
    def read_identifier(self) -> str:
        """Read an identifier or keyword"""
        value = ""
        
        while (self.current_char() and 
               (self.current_char().isalnum() or self.current_char() == '_')):
            value += self.advance()
        
        return value
    
    def read_comment(self) -> str:
        """Read a single-line comment"""
        value = ""
        self.advance()  # consume first /
        self.advance()  # consume second /
        
        while self.current_char() and self.current_char() != '\n':
            value += self.advance()
        
        return value
    
    def add_token(self, token_type: TokenType, value: str):
        """Add a token to the tokens list"""
        token = Token(token_type, value, self.line, self.column - len(value))
        self.tokens.append(token)
    
    def tokenize(self) -> List[Token]:
        """Tokenize the entire source code"""
        while self.position < len(self.source):
            self.skip_whitespace()
            
            char = self.current_char()
            if char is None:
                break
            
            # Newlines
            if char == '\n':
                self.add_token(TokenType.NEWLINE, char)
                self.advance()
                continue
            
            # Comments
            if char == '/' and self.peek_char() == '/':
                comment = self.read_comment()
                self.add_token(TokenType.COMMENT, comment)
                continue
            
            # Strings
            if char in '"\'':
                string_value = self.read_string()
                self.add_token(TokenType.STRING, string_value)
                continue
            
            # Numbers
            if char.isdigit():
                number = self.read_number()
                self.add_token(TokenType.NUMBER, number)
                continue
            
            # Identifiers and keywords
            if char.isalpha() or char == '_':
                identifier = self.read_identifier()
                token_type = self.keywords.get(identifier, TokenType.IDENTIFIER)
                self.add_token(token_type, identifier)
                continue
            
            # Two-character operators
            two_char = char + (self.peek_char() or '')
            if two_char in self.operators:
                self.add_token(self.operators[two_char], two_char)
                self.advance()
                self.advance()
                continue
            
            # Single-character operators
            if char in self.operators:
                self.add_token(self.operators[char], char)
                self.advance()
                continue
            
            # Single-character delimiters
            if char in self.single_chars:
                self.add_token(self.single_chars[char], char)
                self.advance()
                continue
            
            # Unknown character
            self.advance()  # Skip unknown characters
        
        # Add EOF token
        self.add_token(TokenType.EOF, '')
        return self.tokens

def tokenize_orion_code(source: str) -> List[Token]:
    """Convenience function to tokenize Orion source code"""
    lexer = OrionLexer(source)
    return lexer.tokenize()

# Test the lexer
if __name__ == "__main__":
    test_code = '''
    fn main() {
        name = "World"
        out("Hello, " + name + "!")
        
        int x = 42
        int y = x * 2
        out("Result: " + str(y))
    }
    '''
    
    tokens = tokenize_orion_code(test_code)
    for token in tokens:
        if token.type != TokenType.NEWLINE and token.type != TokenType.COMMENT:
            print(f"{token.type.value:15} | {token.value}")