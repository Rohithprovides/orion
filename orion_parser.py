"""
Orion Programming Language Parser
Builds Abstract Syntax Tree from tokens
"""

from typing import List, Optional, Union, Any
from dataclasses import dataclass
from orion_lexer import Token, TokenType, tokenize_orion_code

# AST Node base class
@dataclass
class ASTNode:
    pass

# Expressions
@dataclass
class NumberLiteral(ASTNode):
    value: Union[int, float]

@dataclass
class StringLiteral(ASTNode):
    value: str

@dataclass
class Identifier(ASTNode):
    name: str

@dataclass
class BinaryOperation(ASTNode):
    left: ASTNode
    operator: str
    right: ASTNode

@dataclass
class UnaryOperation(ASTNode):
    operator: str
    operand: ASTNode

@dataclass
class FunctionCall(ASTNode):
    name: str
    arguments: List[ASTNode]

# Statements
@dataclass
class Assignment(ASTNode):
    name: str
    value: ASTNode
    type_annotation: Optional[str] = None

@dataclass
class ExpressionStatement(ASTNode):
    expression: ASTNode

@dataclass
class Block(ASTNode):
    statements: List[ASTNode]

@dataclass
class IfStatement(ASTNode):
    condition: ASTNode
    then_block: Block
    else_block: Optional[Block] = None

@dataclass
class WhileStatement(ASTNode):
    condition: ASTNode
    body: Block

@dataclass
class ForStatement(ASTNode):
    init: Optional[ASTNode]
    condition: Optional[ASTNode]
    update: Optional[ASTNode]
    body: Block

@dataclass
class ReturnStatement(ASTNode):
    value: Optional[ASTNode] = None

@dataclass
class FunctionDeclaration(ASTNode):
    name: str
    parameters: List[tuple]  # [(name, type), ...]
    return_type: Optional[str]
    body: Block

@dataclass
class Program(ASTNode):
    statements: List[ASTNode]

class ParseError(Exception):
    def __init__(self, message: str, token: Optional[Token] = None):
        self.message = message
        self.token = token
        super().__init__(message)

class OrionParser:
    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.position = 0
    
    def current_token(self) -> Optional[Token]:
        """Get the current token"""
        if self.position >= len(self.tokens):
            return None
        return self.tokens[self.position]
    
    def peek_token(self, offset: int = 1) -> Optional[Token]:
        """Peek at the next token without consuming it"""
        pos = self.position + offset
        if pos >= len(self.tokens):
            return None
        return self.tokens[pos]
    
    def advance(self) -> Optional[Token]:
        """Consume and return the current token"""
        token = self.current_token()
        if token and token.type != TokenType.EOF:
            self.position += 1
        return token
    
    def match(self, *token_types: TokenType) -> bool:
        """Check if current token matches any of the given types"""
        current = self.current_token()
        return current and current.type in token_types
    
    def consume(self, token_type: TokenType, message: str = None) -> Token:
        """Consume a token of the expected type or raise an error"""
        token = self.current_token()
        if not token or token.type != token_type:
            error_msg = message or f"Expected {token_type}, got {token.type if token else 'EOF'}"
            raise ParseError(error_msg, token)
        return self.advance()
    
    def skip_newlines(self):
        """Skip newline tokens"""
        while self.match(TokenType.NEWLINE, TokenType.COMMENT):
            self.advance()
    
    def parse(self) -> Program:
        """Parse the entire program"""
        statements = []
        
        while not self.match(TokenType.EOF):
            self.skip_newlines()
            if self.match(TokenType.EOF):
                break
            
            stmt = self.parse_statement()
            if stmt:
                statements.append(stmt)
        
        return Program(statements)
    
    def parse_statement(self) -> Optional[ASTNode]:
        """Parse a statement"""
        self.skip_newlines()
        
        if self.match(TokenType.FN):
            return self.parse_function_declaration()
        elif self.match(TokenType.IF):
            return self.parse_if_statement()
        elif self.match(TokenType.WHILE):
            return self.parse_while_statement()
        elif self.match(TokenType.FOR):
            return self.parse_for_statement()
        elif self.match(TokenType.RETURN):
            return self.parse_return_statement()
        elif self.match(TokenType.LBRACE):
            return self.parse_block()
        else:
            # Try to parse assignment or expression statement
            return self.parse_assignment_or_expression()
    
    def parse_function_declaration(self) -> FunctionDeclaration:
        """Parse function declaration: fn name(params) { body }"""
        self.consume(TokenType.FN)
        
        # Handle both regular identifiers and 'main' keyword
        if self.match(TokenType.IDENTIFIER):
            name_token = self.advance()
            name = name_token.value
        elif self.match(TokenType.MAIN):
            name_token = self.advance()
            name = name_token.value
        else:
            raise ParseError("Expected function name", self.current_token())
        
        self.consume(TokenType.LPAREN, "Expected '(' after function name")
        
        # Parse parameters
        parameters = []
        if not self.match(TokenType.RPAREN):
            while True:
                param_name = self.consume(TokenType.IDENTIFIER, "Expected parameter name").value
                param_type = None
                
                # Check for type annotation
                if self.match(TokenType.INT, TokenType.FLOAT, TokenType.STRING_TYPE, TokenType.BOOL):
                    param_type = self.advance().value
                
                parameters.append((param_name, param_type))
                
                if self.match(TokenType.COMMA):
                    self.advance()
                else:
                    break
        
        self.consume(TokenType.RPAREN, "Expected ')' after parameters")
        
        # Optional return type (not implemented in this basic version)
        return_type = None
        
        body = self.parse_block()
        
        return FunctionDeclaration(name, parameters, return_type, body)
    
    def parse_block(self) -> Block:
        """Parse a block: { statements }"""
        self.consume(TokenType.LBRACE, "Expected '{'")
        
        statements = []
        while not self.match(TokenType.RBRACE, TokenType.EOF):
            self.skip_newlines()
            if self.match(TokenType.RBRACE, TokenType.EOF):
                break
            
            stmt = self.parse_statement()
            if stmt:
                statements.append(stmt)
        
        self.consume(TokenType.RBRACE, "Expected '}'")
        return Block(statements)
    
    def parse_if_statement(self) -> IfStatement:
        """Parse if statement: if condition { then_block } else { else_block }"""
        self.consume(TokenType.IF)
        
        condition = self.parse_expression()
        then_block = self.parse_block()
        
        else_block = None
        if self.match(TokenType.ELSE):
            self.advance()
            else_block = self.parse_block()
        
        return IfStatement(condition, then_block, else_block)
    
    def parse_while_statement(self) -> WhileStatement:
        """Parse while statement: while condition { body }"""
        self.consume(TokenType.WHILE)
        
        condition = self.parse_expression()
        body = self.parse_block()
        
        return WhileStatement(condition, body)
    
    def parse_for_statement(self) -> ForStatement:
        """Parse for statement (basic version)"""
        self.consume(TokenType.FOR)
        
        # Simplified for loop: for init; condition; update { body }
        init = None
        condition = None
        update = None
        
        # This is a simplified version - real implementation would be more complex
        body = self.parse_block()
        
        return ForStatement(init, condition, update, body)
    
    def parse_return_statement(self) -> ReturnStatement:
        """Parse return statement: return expression?"""
        self.consume(TokenType.RETURN)
        
        value = None
        if not self.match(TokenType.NEWLINE, TokenType.RBRACE, TokenType.EOF):
            value = self.parse_expression()
        
        return ReturnStatement(value)
    
    def parse_assignment_or_expression(self) -> ASTNode:
        """Parse assignment or expression statement"""
        # Look ahead to see if this is an assignment
        if (self.match(TokenType.IDENTIFIER) and 
            (self.peek_token() and self.peek_token().type == TokenType.ASSIGN)):
            return self.parse_assignment()
        elif (self.match(TokenType.INT, TokenType.FLOAT, TokenType.STRING_TYPE, TokenType.BOOL) and
              self.peek_token() and self.peek_token().type == TokenType.IDENTIFIER):
            return self.parse_typed_assignment()
        else:
            expr = self.parse_expression()
            return ExpressionStatement(expr)
    
    def parse_assignment(self) -> Assignment:
        """Parse assignment: identifier = expression"""
        name = self.consume(TokenType.IDENTIFIER).value
        self.consume(TokenType.ASSIGN)
        value = self.parse_expression()
        
        return Assignment(name, value)
    
    def parse_typed_assignment(self) -> Assignment:
        """Parse typed assignment: type identifier = expression"""
        type_token = self.advance()  # consume type
        name = self.consume(TokenType.IDENTIFIER).value
        self.consume(TokenType.ASSIGN)
        value = self.parse_expression()
        
        return Assignment(name, value, type_token.value)
    
    def parse_expression(self) -> ASTNode:
        """Parse expression with operator precedence"""
        return self.parse_logical_or()
    
    def parse_logical_or(self) -> ASTNode:
        """Parse logical OR: expr || expr"""
        expr = self.parse_logical_and()
        
        while self.match(TokenType.OR):
            operator = self.advance().value
            right = self.parse_logical_and()
            expr = BinaryOperation(expr, operator, right)
        
        return expr
    
    def parse_logical_and(self) -> ASTNode:
        """Parse logical AND: expr && expr"""
        expr = self.parse_equality()
        
        while self.match(TokenType.AND):
            operator = self.advance().value
            right = self.parse_equality()
            expr = BinaryOperation(expr, operator, right)
        
        return expr
    
    def parse_equality(self) -> ASTNode:
        """Parse equality: expr == expr, expr != expr"""
        expr = self.parse_comparison()
        
        while self.match(TokenType.EQUAL, TokenType.NOT_EQUAL):
            operator = self.advance().value
            right = self.parse_comparison()
            expr = BinaryOperation(expr, operator, right)
        
        return expr
    
    def parse_comparison(self) -> ASTNode:
        """Parse comparison: expr < expr, expr > expr, etc."""
        expr = self.parse_addition()
        
        while self.match(TokenType.LESS_THAN, TokenType.GREATER_THAN, 
                         TokenType.LESS_EQUAL, TokenType.GREATER_EQUAL):
            operator = self.advance().value
            right = self.parse_addition()
            expr = BinaryOperation(expr, operator, right)
        
        return expr
    
    def parse_addition(self) -> ASTNode:
        """Parse addition and subtraction: expr + expr, expr - expr"""
        expr = self.parse_multiplication()
        
        while self.match(TokenType.PLUS, TokenType.MINUS):
            operator = self.advance().value
            right = self.parse_multiplication()
            expr = BinaryOperation(expr, operator, right)
        
        return expr
    
    def parse_multiplication(self) -> ASTNode:
        """Parse multiplication, division, modulo: expr * expr, expr / expr, expr % expr"""
        expr = self.parse_unary()
        
        while self.match(TokenType.MULTIPLY, TokenType.DIVIDE, TokenType.MODULO):
            operator = self.advance().value
            right = self.parse_unary()
            expr = BinaryOperation(expr, operator, right)
        
        return expr
    
    def parse_unary(self) -> ASTNode:
        """Parse unary expressions: -expr, !expr"""
        if self.match(TokenType.MINUS, TokenType.NOT):
            operator = self.advance().value
            operand = self.parse_unary()
            return UnaryOperation(operator, operand)
        
        return self.parse_primary()
    
    def parse_primary(self) -> ASTNode:
        """Parse primary expressions: literals, identifiers, function calls, parentheses"""
        if self.match(TokenType.NUMBER):
            token = self.advance()
            value = int(token.value) if '.' not in token.value else float(token.value)
            return NumberLiteral(value)
        
        if self.match(TokenType.STRING):
            value = self.advance().value
            return StringLiteral(value)
        
        if self.match(TokenType.IDENTIFIER, TokenType.OUT):
            name = self.advance().value
            
            # Check for function call
            if self.match(TokenType.LPAREN):
                self.advance()  # consume '('
                
                arguments = []
                if not self.match(TokenType.RPAREN):
                    while True:
                        arg = self.parse_expression()
                        arguments.append(arg)
                        
                        if self.match(TokenType.COMMA):
                            self.advance()
                        else:
                            break
                
                self.consume(TokenType.RPAREN, "Expected ')' after function arguments")
                return FunctionCall(name, arguments)
            else:
                return Identifier(name)
        
        if self.match(TokenType.LPAREN):
            self.advance()  # consume '('
            expr = self.parse_expression()
            self.consume(TokenType.RPAREN, "Expected ')' after expression")
            return expr
        
        token = self.current_token()
        raise ParseError(f"Unexpected token: {token.value if token else 'EOF'}", token)

def parse_orion_code(source: str) -> Program:
    """Convenience function to parse Orion source code"""
    tokens = tokenize_orion_code(source)
    parser = OrionParser(tokens)
    return parser.parse()

# Test the parser
if __name__ == "__main__":
    test_code = '''
    fn main() {
        name = "World"
        out("Hello, " + name + "!")
        
        int x = 42
        y = x * 2
        out("Result: " + str(y))
    }
    '''
    
    try:
        ast = parse_orion_code(test_code)
        print("AST parsed successfully!")
        print(f"Program has {len(ast.statements)} top-level statements")
        for i, stmt in enumerate(ast.statements):
            print(f"Statement {i+1}: {type(stmt).__name__}")
    except ParseError as e:
        print(f"Parse error: {e.message}")
        if e.token:
            print(f"At line {e.token.line}, column {e.token.column}")