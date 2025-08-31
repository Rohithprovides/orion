"""
Orion Programming Language Interpreter
Executes Orion Abstract Syntax Tree
"""

from typing import Any, Dict, List, Optional, IO
import sys
from orion_lexer import tokenize_orion_code
from orion_parser import (
    parse_orion_code, ASTNode, Program, FunctionDeclaration, Block,
    Assignment, ExpressionStatement, IfStatement, WhileStatement, ForStatement,
    ReturnStatement, BinaryOperation, UnaryOperation, FunctionCall,
    NumberLiteral, StringLiteral, Identifier, ParseError
)

class RuntimeError(Exception):
    def __init__(self, message: str):
        self.message = message
        super().__init__(message)

class ReturnValue(Exception):
    """Exception used to implement return statements"""
    def __init__(self, value: Any):
        self.value = value

class Environment:
    """Variable scope environment"""
    def __init__(self, parent: Optional['Environment'] = None):
        self.parent = parent
        self.variables: Dict[str, Any] = {}
    
    def define(self, name: str, value: Any):
        """Define a variable in this environment"""
        self.variables[name] = value
    
    def get(self, name: str) -> Any:
        """Get a variable value, checking parent scopes if needed"""
        if name in self.variables:
            return self.variables[name]
        elif self.parent:
            return self.parent.get(name)
        else:
            raise RuntimeError(f"Undefined variable: {name}")
    
    def set(self, name: str, value: Any):
        """Set a variable value, checking parent scopes if needed"""
        if name in self.variables:
            self.variables[name] = value
        elif self.parent and self.parent.has(name):
            self.parent.set(name, value)
        else:
            # Define in current scope if it doesn't exist
            self.variables[name] = value
    
    def has(self, name: str) -> bool:
        """Check if a variable exists in this environment or parent scopes"""
        return name in self.variables or (self.parent and self.parent.has(name))

class OrionInterpreter:
    def __init__(self, output_stream: IO = None):
        self.global_env = Environment()
        self.current_env = self.global_env
        self.functions: Dict[str, FunctionDeclaration] = {}
        self.output_stream = output_stream or sys.stdout
        self.output_buffer: List[str] = []
        
        # Built-in functions
        self._setup_builtins()
    
    def _setup_builtins(self):
        """Setup built-in functions"""
        # Built-in functions are handled specially in function calls
        pass
    
    def capture_output(self) -> str:
        """Get captured output as string"""
        return ''.join(self.output_buffer)
    
    def clear_output(self):
        """Clear the output buffer"""
        self.output_buffer.clear()
    
    def _print_to_output(self, text: str):
        """Print to output stream and capture in buffer"""
        self.output_buffer.append(text)
        if self.output_stream:
            self.output_stream.write(text)
            self.output_stream.flush()
    
    def execute(self, ast: Program) -> Any:
        """Execute the entire program"""
        try:
            # First pass: collect function declarations
            for stmt in ast.statements:
                if isinstance(stmt, FunctionDeclaration):
                    self.functions[stmt.name] = stmt
            
            # Look for main function and execute it
            if 'main' in self.functions:
                return self._call_function('main', [])
            else:
                # Execute statements in order if no main function
                result = None
                for stmt in ast.statements:
                    if not isinstance(stmt, FunctionDeclaration):
                        result = self._execute_statement(stmt)
                return result
                
        except ReturnValue as ret:
            return ret.value
        except Exception as e:
            raise RuntimeError(f"Runtime error: {str(e)}")
    
    def _execute_statement(self, node: ASTNode) -> Any:
        """Execute a statement"""
        if isinstance(node, Assignment):
            return self._execute_assignment(node)
        elif isinstance(node, ExpressionStatement):
            return self._execute_expression(node.expression)
        elif isinstance(node, Block):
            return self._execute_block(node)
        elif isinstance(node, IfStatement):
            return self._execute_if(node)
        elif isinstance(node, WhileStatement):
            return self._execute_while(node)
        elif isinstance(node, ForStatement):
            return self._execute_for(node)
        elif isinstance(node, ReturnStatement):
            return self._execute_return(node)
        elif isinstance(node, FunctionDeclaration):
            # Function declarations are handled in the first pass
            return None
        else:
            raise RuntimeError(f"Unknown statement type: {type(node)}")
    
    def _execute_assignment(self, node: Assignment) -> None:
        """Execute assignment statement"""
        value = self._execute_expression(node.value)
        
        # Handle type conversion based on type annotation
        if node.type_annotation:
            if node.type_annotation == 'int':
                value = int(value) if isinstance(value, (int, float, str)) else value
            elif node.type_annotation == 'float':
                value = float(value) if isinstance(value, (int, float, str)) else value
            elif node.type_annotation == 'string':
                value = str(value)
            elif node.type_annotation == 'bool':
                value = bool(value)
        
        self.current_env.set(node.name, value)
    
    def _execute_block(self, node: Block) -> Any:
        """Execute a block of statements"""
        # Create new scope
        previous_env = self.current_env
        self.current_env = Environment(previous_env)
        
        try:
            result = None
            for stmt in node.statements:
                result = self._execute_statement(stmt)
            return result
        finally:
            # Restore previous scope
            self.current_env = previous_env
    
    def _execute_if(self, node: IfStatement) -> Any:
        """Execute if statement"""
        condition = self._execute_expression(node.condition)
        
        if self._is_truthy(condition):
            return self._execute_statement(node.then_block)
        elif node.else_block:
            return self._execute_statement(node.else_block)
        
        return None
    
    def _execute_while(self, node: WhileStatement) -> Any:
        """Execute while loop"""
        result = None
        while self._is_truthy(self._execute_expression(node.condition)):
            result = self._execute_statement(node.body)
        return result
    
    def _execute_for(self, node: ForStatement) -> Any:
        """Execute for loop (simplified implementation)"""
        # This is a placeholder - real implementation would handle init, condition, update
        return self._execute_statement(node.body)
    
    def _execute_return(self, node: ReturnStatement) -> None:
        """Execute return statement"""
        value = None
        if node.value:
            value = self._execute_expression(node.value)
        raise ReturnValue(value)
    
    def _execute_expression(self, node: ASTNode) -> Any:
        """Execute an expression and return its value"""
        if isinstance(node, NumberLiteral):
            return node.value
        elif isinstance(node, StringLiteral):
            return node.value
        elif isinstance(node, Identifier):
            return self.current_env.get(node.name)
        elif isinstance(node, BinaryOperation):
            return self._execute_binary_operation(node)
        elif isinstance(node, UnaryOperation):
            return self._execute_unary_operation(node)
        elif isinstance(node, FunctionCall):
            return self._execute_function_call(node)
        else:
            raise RuntimeError(f"Unknown expression type: {type(node)}")
    
    def _execute_binary_operation(self, node: BinaryOperation) -> Any:
        """Execute binary operation"""
        left = self._execute_expression(node.left)
        right = self._execute_expression(node.right)
        
        if node.operator == '+':
            return left + right
        elif node.operator == '-':
            return left - right
        elif node.operator == '*':
            return left * right
        elif node.operator == '/':
            if right == 0:
                raise RuntimeError("Division by zero")
            return left / right
        elif node.operator == '%':
            return left % right
        elif node.operator == '==':
            return left == right
        elif node.operator == '!=':
            return left != right
        elif node.operator == '<':
            return left < right
        elif node.operator == '>':
            return left > right
        elif node.operator == '<=':
            return left <= right
        elif node.operator == '>=':
            return left >= right
        elif node.operator == 'and':
            return self._is_truthy(left) and self._is_truthy(right)
        elif node.operator == 'or':
            return self._is_truthy(left) or self._is_truthy(right)
        else:
            raise RuntimeError(f"Unknown binary operator: {node.operator}")
    
    def _execute_unary_operation(self, node: UnaryOperation) -> Any:
        """Execute unary operation"""
        operand = self._execute_expression(node.operand)
        
        if node.operator == '-':
            return -operand
        elif node.operator == 'not':
            return not self._is_truthy(operand)
        else:
            raise RuntimeError(f"Unknown unary operator: {node.operator}")
    
    def _execute_function_call(self, node: FunctionCall) -> Any:
        """Execute function call"""
        # Handle built-in functions
        if node.name == 'out':
            return self._builtin_out(node.arguments)
        elif node.name == 'str':
            return self._builtin_str(node.arguments)
        elif node.name == 'int':
            return self._builtin_int(node.arguments)
        elif node.name == 'float':
            return self._builtin_float(node.arguments)
        else:
            # User-defined function
            return self._call_function(node.name, node.arguments)
    
    def _call_function(self, name: str, arguments: List[ASTNode]) -> Any:
        """Call a user-defined function"""
        if name not in self.functions:
            raise RuntimeError(f"Undefined function: {name}")
        
        func = self.functions[name]
        
        # Evaluate arguments
        arg_values = [self._execute_expression(arg) for arg in arguments]
        
        # Check argument count
        if len(arg_values) != len(func.parameters):
            raise RuntimeError(f"Function {name} expects {len(func.parameters)} arguments, got {len(arg_values)}")
        
        # Create new environment for function scope
        previous_env = self.current_env
        self.current_env = Environment(self.global_env)
        
        # Bind parameters
        for (param_name, param_type), arg_value in zip(func.parameters, arg_values):
            # Apply type conversion if specified
            if param_type:
                if param_type == 'int':
                    arg_value = int(arg_value)
                elif param_type == 'float':
                    arg_value = float(arg_value)
                elif param_type == 'string':
                    arg_value = str(arg_value)
                elif param_type == 'bool':
                    arg_value = bool(arg_value)
            
            self.current_env.define(param_name, arg_value)
        
        try:
            # Execute function body
            self._execute_statement(func.body)
            return None  # No explicit return
        except ReturnValue as ret:
            return ret.value
        finally:
            # Restore previous environment
            self.current_env = previous_env
    
    def _builtin_out(self, arguments: List[ASTNode]) -> None:
        """Built-in out() function"""
        if len(arguments) != 1:
            raise RuntimeError("out() expects exactly 1 argument")
        
        value = self._execute_expression(arguments[0])
        text = str(value)
        self._print_to_output(text + '\n')
    
    def _builtin_str(self, arguments: List[ASTNode]) -> str:
        """Built-in str() function"""
        if len(arguments) != 1:
            raise RuntimeError("str() expects exactly 1 argument")
        
        value = self._execute_expression(arguments[0])
        return str(value)
    
    def _builtin_int(self, arguments: List[ASTNode]) -> int:
        """Built-in int() function"""
        if len(arguments) != 1:
            raise RuntimeError("int() expects exactly 1 argument")
        
        value = self._execute_expression(arguments[0])
        return int(value)
    
    def _builtin_float(self, arguments: List[ASTNode]) -> float:
        """Built-in float() function"""
        if len(arguments) != 1:
            raise RuntimeError("float() expects exactly 1 argument")
        
        value = self._execute_expression(arguments[0])
        return float(value)
    
    def _is_truthy(self, value: Any) -> bool:
        """Determine if a value is truthy"""
        if value is None:
            return False
        if isinstance(value, bool):
            return value
        if isinstance(value, (int, float)):
            return value != 0
        if isinstance(value, str):
            return len(value) > 0
        return True

def execute_orion_code(source: str) -> tuple[Any, str]:
    """Execute Orion source code and return result and output"""
    try:
        ast = parse_orion_code(source)
        interpreter = OrionInterpreter()
        result = interpreter.execute(ast)
        output = interpreter.capture_output()
        return result, output
    except (ParseError, RuntimeError) as e:
        raise e

# Test the interpreter
if __name__ == "__main__":
    test_code = '''
    fn main() {
        name = "World"
        out("Hello, " + name + "!")
        
        int x = 42
        y = x * 2
        out("Result: " + str(y))
        
        if x > 40 {
            out("x is greater than 40")
        }
        
        i = 0
        while i < 3 {
            out("Count: " + str(i))
            i = i + 1
        }
    }
    '''
    
    try:
        result, output = execute_orion_code(test_code)
        print("Execution completed successfully!")
        print("Output:")
        print(output)
        print(f"Return value: {result}")
    except Exception as e:
        print(f"Error: {e}")