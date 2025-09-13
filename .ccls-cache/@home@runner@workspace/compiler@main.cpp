#include "ast.h"
#include "lexer.h"
#include "simple_parser.h"
#include "types.cpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_set>

namespace orion {


// Simplified Code Generator for basic functionality
class SimpleCodeGenerator : public ASTVisitor {
private:
    std::ostringstream assembly;
    std::vector<std::string> stringLiterals;
    std::vector<double> floatLiterals;
    struct VariableInfo {
        int stackOffset;
        std::string type;
        bool isGlobal;
        bool isConstant;
    };
    std::unordered_map<std::string, VariableInfo> globalVariables; // Global scope variables
    std::unordered_map<std::string, VariableInfo> localVariables; // Current function scope variables
    std::unordered_set<std::string> declaredGlobal; // Variables explicitly declared global with 'global' keyword
    std::unordered_set<std::string> declaredLocal;  // Variables explicitly declared local with 'local' keyword
    std::unordered_set<std::string> constantVariables; // Variables declared as const
    
    // Hierarchical function storage for proper scoping
    struct FunctionScope {
        std::unordered_map<std::string, FunctionDeclaration*> functions;
        std::string parentFunction; // Name of parent function (empty for global scope)
    };
    
    std::unordered_map<std::string, FunctionScope> functionScopes; // Scope name -> functions in that scope
    std::vector<std::string> functionCallStack; // Track current function execution stack
    
    int stackOffset = 0;
    bool inFunction = false;
    int labelCounter = 0;
    
    std::string newLabel(const std::string& prefix = "L") {
        return prefix + std::to_string(labelCounter++);
    }
    
    bool isFloatExpression(Expression* expr) {
        if (auto floatLit = dynamic_cast<FloatLiteral*>(expr)) {
            return true;
        }
        if (auto id = dynamic_cast<Identifier*>(expr)) {
            auto varInfo = lookupVariable(id->name);
            return varInfo && varInfo->type == "float";
        }
        return false;
    }
    
    // Expression kind inference for type safety
    enum class ExprKind { INT, FLOAT, BOOL, STRING, LIST, UNKNOWN };
    
    ExprKind inferExprKind(Expression* expr) {
        if (auto intLit = dynamic_cast<IntLiteral*>(expr)) {
            return ExprKind::INT;
        }
        if (auto floatLit = dynamic_cast<FloatLiteral*>(expr)) {
            return ExprKind::FLOAT;
        }
        if (auto boolLit = dynamic_cast<BoolLiteral*>(expr)) {
            return ExprKind::BOOL;
        }
        if (auto strLit = dynamic_cast<StringLiteral*>(expr)) {
            return ExprKind::STRING;
        }
        if (auto listLit = dynamic_cast<ListLiteral*>(expr)) {
            return ExprKind::LIST;
        }
        if (auto id = dynamic_cast<Identifier*>(expr)) {
            auto var = lookupVariable(id->name);
            if (var) {
                if (var->type == "int") return ExprKind::INT;
                if (var->type == "float") return ExprKind::FLOAT;
                if (var->type == "bool") return ExprKind::BOOL;
                if (var->type == "string") return ExprKind::STRING;
                if (var->type == "list") return ExprKind::LIST;
            }
        }
        if (auto binExpr = dynamic_cast<BinaryExpression*>(expr)) {
            ExprKind leftKind = inferExprKind(binExpr->left.get());
            ExprKind rightKind = inferExprKind(binExpr->right.get());
            
            if (binExpr->op == BinaryOp::ADD) {
                if (leftKind == ExprKind::LIST && rightKind == ExprKind::LIST) {
                    return ExprKind::LIST;  // List concatenation
                }
            }
            if (binExpr->op == BinaryOp::MUL) {
                if ((leftKind == ExprKind::LIST && rightKind == ExprKind::INT) ||
                    (leftKind == ExprKind::INT && rightKind == ExprKind::LIST)) {
                    return ExprKind::LIST;  // List repetition
                }
            }
            
            // Default to numeric operations
            if (leftKind == ExprKind::FLOAT || rightKind == ExprKind::FLOAT) {
                return ExprKind::FLOAT;
            }
            return ExprKind::INT;
        }
        return ExprKind::UNKNOWN;
    }
    
    int addStringLiteral(const std::string& str) {
        stringLiterals.push_back(str);
        return stringLiterals.size() - 1;
    }
    
    int addFloatLiteral(double value) {
        floatLiterals.push_back(value);
        return floatLiterals.size() - 1;
    }
    
public:
    std::string generate(Program& program) {
        assembly.str("");
        assembly.clear();
        stringLiterals.clear();
        floatLiterals.clear();
        globalVariables.clear();
        localVariables.clear();
        declaredGlobal.clear();
        declaredLocal.clear();
        constantVariables.clear();
        inFunction = false;
        stackOffset = 0;
        labelCounter = 0;
        
        // Visit program to collect strings and generate code
        program.accept(*this);
        
        // Generate complete assembly
        std::ostringstream fullAssembly;
        
        // Data section
        fullAssembly << ".section .data\n";
        fullAssembly << "format_int: .string \"%d\\n\"\n";
        fullAssembly << "format_str: .string \"%s\"\n";
        fullAssembly << "format_float: .string \"%.2f\\n\"\n";
        fullAssembly << "dtype_int: .string \"datatype: int\\n\"\n";
        fullAssembly << "dtype_string: .string \"datatype: string\\n\"\n";
        fullAssembly << "dtype_bool: .string \"datatype: bool\\n\"\n";
        fullAssembly << "dtype_float: .string \"datatype: float\\n\"\n";
        fullAssembly << "dtype_list: .string \"datatype: list\\n\"\n";
        fullAssembly << "dtype_unknown: .string \"datatype: unknown\\n\"\n";
        fullAssembly << "str_true: .string \"True\\n\"\n";
        fullAssembly << "str_false: .string \"False\\n\"\n";
        fullAssembly << "str_index_error: .string \"Index Error\\n\"\n";
        
        // String literals
        for (size_t i = 0; i < stringLiterals.size(); i++) {
            fullAssembly << "str_" << i << ": .string \"" << stringLiterals[i] << "\\n\"\n";
        }
        
        // Add float literals  
        for (size_t i = 0; i < floatLiterals.size(); ++i) {
            fullAssembly << "float_" << i << ": .quad " << *reinterpret_cast<uint64_t*>(&floatLiterals[i]) << "\n";
        }
        
        // Text section
        fullAssembly << "\n.section .text\n";
        fullAssembly << ".global main\n";
        fullAssembly << ".extern printf\n";
        fullAssembly << ".extern orion_malloc\n";
        fullAssembly << ".extern orion_free\n";
        fullAssembly << ".extern exit\n";
        fullAssembly << ".extern fmod\n";
        fullAssembly << ".extern pow\n";
        // Enhanced list runtime functions
        fullAssembly << ".extern list_new\n";
        fullAssembly << ".extern list_from_data\n";
        fullAssembly << ".extern list_len\n";
        fullAssembly << ".extern list_get\n";
        fullAssembly << ".extern list_set\n";
        fullAssembly << ".extern list_append\n";
        fullAssembly << ".extern list_pop\n";
        fullAssembly << ".extern list_insert\n";
        fullAssembly << ".extern list_concat\n";
        fullAssembly << ".extern list_repeat\n";
        fullAssembly << ".extern list_extend\n\n";
        
        // Main function
        fullAssembly << "main:\n";
        fullAssembly << "    push %rbp\n";
        fullAssembly << "    mov %rsp, %rbp\n";
        fullAssembly << "    sub $64, %rsp\n";  // Allocate 64 bytes of stack space for variables
        
        // Program code
        fullAssembly << assembly.str();
        
        // Return 0
        fullAssembly << "    mov $0, %rax\n";
        fullAssembly << "    add $64, %rsp\n";  // Restore stack pointer
        fullAssembly << "    pop %rbp\n";
        fullAssembly << "    ret\n";
        
        return fullAssembly.str();
    }
    
    void visit(Program& node) override {
        // First pass: collect all function definitions with proper scoping
        collectFunctions(node.statements, ""); // Start with global scope
        
        // Second pass: execute only non-function statements and function calls
        for (auto& stmt : node.statements) {
            if (dynamic_cast<FunctionDeclaration*>(stmt.get()) == nullptr) {
                stmt->accept(*this);
            }
        }
        
        // Note: main() functions now only execute when explicitly called (Python-style behavior)
    }
    
    void collectFunctions(const std::vector<std::unique_ptr<Statement>>& statements, const std::string& currentScope = "") {
        for (auto& stmt : statements) {
            if (auto func = dynamic_cast<FunctionDeclaration*>(stmt.get())) {
                // Store function definition in the appropriate scope
                if (functionScopes.find(currentScope) == functionScopes.end()) {
                    functionScopes[currentScope] = FunctionScope{};
                }
                
                functionScopes[currentScope].functions[func->name] = func;
                assembly << "    # Function '" << func->name << "' defined in scope '" << currentScope << "'\n";
                
                // Recursively collect nested functions from this function's body with proper scope
                if (!func->isSingleExpression) {
                    std::string nestedScope = currentScope.empty() ? func->name : currentScope + "::" + func->name;
                    collectFunctions(func->body, nestedScope);
                }
            } else if (auto block = dynamic_cast<BlockStatement*>(stmt.get())) {
                // Recursively collect nested functions in same scope
                collectFunctions(block->statements, currentScope);
            }
        }
    }
    
    void visit(FunctionDeclaration& node) override {
        // Functions are only executed when called, not when defined
        assembly << "    # Function '" << node.name << "' defined but not executed\n";
        
        // Store function in our function table (simplified - we store the raw pointer)
        // In a real implementation, we'd need proper cloning/copying
        // For now, just mark that we've seen this function
    }
    
    void executeFunctionCall(const std::string& functionName, const std::vector<std::unique_ptr<Expression>>& arguments) {
        assembly << "    # Executing function call: " << functionName << "\n";
        
        // Find the function in the current scope
        FunctionDeclaration* func = findFunction(functionName);
        if (!func) {
            throw std::runtime_error("Error: Undefined function '" + functionName + "' in current scope");
        }
        
        // Save current function state
        bool wasInFunction = inFunction;
        auto savedLocalVars = localVariables;
        auto savedDeclaredGlobal = declaredGlobal;
        auto savedDeclaredLocal = declaredLocal;
        
        // Push function onto call stack for proper scoping
        std::string currentScope = "";
        if (!functionCallStack.empty()) {
            currentScope = functionCallStack.back();
        }
        std::string newScope = currentScope.empty() ? functionName : currentScope + "::" + functionName;
        functionCallStack.push_back(newScope);
        
        // Enter new function scope
        inFunction = true;
        localVariables.clear(); // Clear local variables for new function scope
        declaredGlobal.clear(); // Clear global declarations for new function
        declaredLocal.clear(); // Clear local declarations for new function
        
        // Execute function body
        if (func->isSingleExpression) {
            func->expression->accept(*this);
        } else {
            for (auto& stmt : func->body) {
                stmt->accept(*this);
            }
        }
        
        // Exit function scope - restore previous state and pop call stack
        functionCallStack.pop_back();
        inFunction = wasInFunction;
        localVariables = savedLocalVars; // Restore previous local scope
        declaredGlobal = savedDeclaredGlobal;
        declaredLocal = savedDeclaredLocal;
    }
    
    FunctionDeclaration* findFunction(const std::string& name) {
        // Implement Python-style function scoping: look in current scope, then parent scopes, then global
        
        // Start from current function context (if any)
        std::string currentScope = "";
        if (!functionCallStack.empty()) {
            currentScope = functionCallStack.back();
        }
        
        // Search in current scope hierarchy (from innermost to outermost)
        std::string searchScope = currentScope;
        while (true) {
            auto scopeIt = functionScopes.find(searchScope);
            if (scopeIt != functionScopes.end()) {
                auto funcIt = scopeIt->second.functions.find(name);
                if (funcIt != scopeIt->second.functions.end()) {
                    return funcIt->second;
                }
            }
            
            // Move to parent scope
            if (searchScope.empty()) {
                break; // Already at global scope
            }
            
            // Extract parent scope (remove last "::function" part)
            size_t pos = searchScope.find_last_of("::");
            if (pos == std::string::npos) {
                searchScope = ""; // Move to global scope
            } else {
                searchScope = searchScope.substr(0, pos-1); // Remove "::function" part
            }
        }
        
        return nullptr; // Function not found in any accessible scope
    }
    
    void visit(VariableDeclaration& node) override {
        assembly << "    # Variable: " << node.name << "\n";
        
        // Check if this is an assignment to an existing const variable
        if (!node.isConstant && constantVariables.count(node.name)) {
            throw std::runtime_error("Error: You are trying to change the value of a constant variable '" + node.name + "'");
        }
        
        if (node.initializer) {
            // Determine variable type from initializer
            std::string varType = "unknown";
            if (auto intLit = dynamic_cast<IntLiteral*>(node.initializer.get())) {
                varType = "int";
            } else if (auto strLit = dynamic_cast<StringLiteral*>(node.initializer.get())) {
                varType = "string";
            } else if (auto boolLit = dynamic_cast<BoolLiteral*>(node.initializer.get())) {
                varType = "bool";
            } else if (auto floatLit = dynamic_cast<FloatLiteral*>(node.initializer.get())) {
                varType = "float";
            } else if (auto listLit = dynamic_cast<ListLiteral*>(node.initializer.get())) {
                varType = "list";
            } else if (auto id = dynamic_cast<Identifier*>(node.initializer.get())) {
                // Variable assignment: copy type from source variable
                auto varInfo = lookupVariable(id->name);
                if (varInfo != nullptr) {
                    varType = varInfo->type;
                }
            } else if (auto binExpr = dynamic_cast<BinaryExpression*>(node.initializer.get())) {
                // Binary expression: infer type from operands
                // For arithmetic operations, the result is typically int
                switch (binExpr->op) {
                    case BinaryOp::ADD:
                    case BinaryOp::SUB:
                    case BinaryOp::MUL:
                    case BinaryOp::DIV:
                    case BinaryOp::MOD:
                    case BinaryOp::FLOOR_DIV:
                    case BinaryOp::POWER:
                        // Check if either operand is a float
                        if (isFloatExpression(binExpr->left.get()) || isFloatExpression(binExpr->right.get())) {
                            varType = "float";
                        } else {
                            varType = "int";
                        }
                        break;
                    case BinaryOp::EQ:
                    case BinaryOp::NE:
                    case BinaryOp::LT:
                    case BinaryOp::LE:
                    case BinaryOp::GT:
                    case BinaryOp::GE:
                    case BinaryOp::AND:
                    case BinaryOp::OR:
                        varType = "bool";  // Comparison/logical operations result in bool
                        break;
                    default:
                        varType = "int";  // Default to int for unknown operations
                        break;
                }
            }
            
            // Python-style scoping rules - PRE-DECLARE variable before evaluating initializer
            if (declaredGlobal.count(node.name) || (!inFunction)) {
                // Explicitly declared global OR not in function - use global scope
                stackOffset += 8;
                VariableInfo varInfo;
                varInfo.stackOffset = stackOffset;
                varInfo.type = varType;
                varInfo.isGlobal = true;
                varInfo.isConstant = node.isConstant;
                globalVariables[node.name] = varInfo;
                
                if (node.isConstant) {
                    constantVariables.insert(node.name);
                }
            } else {
                // In function and not declared global - create local variable
                stackOffset += 8;
                VariableInfo varInfo;
                varInfo.stackOffset = stackOffset;
                varInfo.type = varType;
                varInfo.isGlobal = false;
                varInfo.isConstant = node.isConstant;
                localVariables[node.name] = varInfo;
                
                if (node.isConstant) {
                    constantVariables.insert(node.name);
                }
            }
            
            // Now evaluate initializer - variable is already declared
            node.initializer->accept(*this);
            
            // Store the result in the pre-allocated variable slot using recorded offset
            VariableInfo* varInfo = lookupVariable(node.name);
            if (varInfo != nullptr) {
                assembly << "    mov %rax, -" << varInfo->stackOffset << "(%rbp)  # store " << (varInfo->isGlobal ? "global" : "local") << " " << node.name << "\n";
            }
        }
    }
    
    VariableInfo* lookupVariable(const std::string& name) {
        // Python-style variable lookup: local scope first, then global scope
        if (inFunction) {
            auto localIt = localVariables.find(name);
            if (localIt != localVariables.end()) {
                return &localIt->second;
            }
        }
        
        auto globalIt = globalVariables.find(name);
        if (globalIt != globalVariables.end()) {
            return &globalIt->second;
        }
        
        return nullptr; // Variable not found
    }
    
    void visit(ExpressionStatement& node) override {
        node.expression->accept(*this);
    }
    
    void visit(FunctionCall& node) override {
        // Handle built-in list functions
        if (node.name == "len") {
            if (node.arguments.size() != 1) {
                throw std::runtime_error("len() function requires exactly 1 argument");
            }
            assembly << "    # len() function call\n";
            node.arguments[0]->accept(*this);  // Evaluate list argument
            assembly << "    mov %rax, %rdi  # List pointer as argument\n";
            assembly << "    call list_len  # Get list length\n";
            // Result in %rax
            return;
        }
        
        if (node.name == "append") {
            if (node.arguments.size() != 2) {
                throw std::runtime_error("append() function requires exactly 2 arguments (list, element)");
            }
            assembly << "    # append() function call\n";
            
            // Evaluate list argument
            node.arguments[0]->accept(*this);
            assembly << "    mov %rax, %rdi  # List pointer as first argument\n";
            assembly << "    push %rdi  # Save list pointer\n";
            
            // Evaluate element argument
            node.arguments[1]->accept(*this);
            assembly << "    mov %rax, %rsi  # Element value as second argument\n";
            assembly << "    pop %rdi  # Restore list pointer\n";
            
            assembly << "    call list_append  # Append element to list\n";
            // append returns void, so no return value
            return;
        }
        
        if (node.name == "pop") {
            if (node.arguments.size() != 1) {
                throw std::runtime_error("pop() function requires exactly 1 argument");
            }
            assembly << "    # pop() function call\n";
            node.arguments[0]->accept(*this);  // Evaluate list argument
            assembly << "    mov %rax, %rdi  # List pointer as argument\n";
            assembly << "    call list_pop  # Pop last element\n";
            // Result (popped element) in %rax
            return;
        }
        
        if (node.name == "out") {
            // Handle out() function calls
            if (!node.arguments.empty()) {
                auto& arg = node.arguments[0];
                
                // Check if the argument is a dtype() function call
                if (auto dtypeCall = dynamic_cast<FunctionCall*>(arg.get())) {
                    if (dtypeCall->name == "dtype" && !dtypeCall->arguments.empty()) {
                        // Handle dtype() call inside out()
                        auto dtypeArg = dtypeCall->arguments[0].get();
                        if (auto id = dynamic_cast<Identifier*>(dtypeArg)) {
                            auto varIt = lookupVariable(id->name);
                            if (varIt != nullptr) {
                                assembly << "    # Call out(dtype(" << id->name << "))\n";
                                std::string dtypeLabel;
                                if (varIt->type == "int") {
                                    dtypeLabel = "dtype_int";
                                } else if (varIt->type == "string") {
                                    dtypeLabel = "dtype_string";
                                } else if (varIt->type == "bool") {
                                    dtypeLabel = "dtype_bool";
                                } else if (varIt->type == "float") {
                                    dtypeLabel = "dtype_float";
                                } else if (varIt->type == "list") {
                                    dtypeLabel = "dtype_list";
                                } else {
                                    dtypeLabel = "dtype_unknown";
                                }
                                assembly << "    mov $" << dtypeLabel << ", %rsi\n";
                                assembly << "    mov $format_str, %rdi\n";
                                assembly << "    xor %rax, %rax\n";
                                assembly << "    call printf\n";
                            } else {
                                throw std::runtime_error("Line " + std::to_string(id->line) + ": Error: Undefined variable '" + id->name + "'");
                            }
                        }
                        return;
                    }
                }
                
                // Check the type of argument to determine format
                if (auto intLit = dynamic_cast<IntLiteral*>(arg.get())) {
                    assembly << "    # Call out() with integer\n";
                    assembly << "    mov $" << intLit->value << ", %rsi\n";
                    assembly << "    mov $format_int, %rdi\n";
                    assembly << "    xor %rax, %rax\n";
                    assembly << "    call printf\n";
                } else if (auto strLit = dynamic_cast<StringLiteral*>(arg.get())) {
                    int index = addStringLiteral(strLit->value);
                    assembly << "    # Call out() with string\n";
                    assembly << "    mov $str_" << index << ", %rsi\n";
                    assembly << "    mov $format_str, %rdi\n";
                    assembly << "    xor %rax, %rax\n";
                    assembly << "    call printf\n";
                } else if (auto id = dynamic_cast<Identifier*>(arg.get())) {
                    // Variable reference - use correct format based on type
                    auto it = lookupVariable(id->name);
                    if (it != nullptr) {
                        assembly << "    # Call out() with variable: " << id->name << " (type: " << it->type << ")\n";
                        assembly << "    mov -" << it->stackOffset << "(%rbp), %rsi\n";
                        
                        if (it->type == "int") {
                            assembly << "    mov $format_int, %rdi\n";
                        } else if (it->type == "bool") {
                            assembly << "    mov $format_str, %rdi\n";
                        } else if (it->type == "float") {
                            assembly << "    movq -" << it->stackOffset << "(%rbp), %xmm0\n";  // Load float into XMM register  
                            assembly << "    mov $format_float, %rdi\n";
                            assembly << "    mov $1, %rax\n";  // Number of vector registers used
                        } else {
                            assembly << "    mov $format_str, %rdi\n";
                        }
                        
                        assembly << "    call printf\n";
                    } else {
                        throw std::runtime_error("Error: Undefined variable '" + id->name + "'");
                    }
                } else if (auto boolLit = dynamic_cast<BoolLiteral*>(arg.get())) {
                    // Boolean literal - output as string
                    assembly << "    # Call out() with boolean literal\n";
                    assembly << "    mov $" << (boolLit->value ? "str_true" : "str_false") << ", %rsi\n";
                    assembly << "    mov $format_str, %rdi\n";
                    assembly << "    xor %rax, %rax\n";
                    assembly << "    call printf\n";
                } else {
                    // Generic expression (like arithmetic operations or comparisons)
                    // Check if the expression result is a float
                    bool isFloatResult = false;
                    bool isComparisonResult = false;
                    
                    if (auto binExpr = dynamic_cast<BinaryExpression*>(arg.get())) {
                        // Check if it's a comparison or logical operation
                        if (binExpr->op == BinaryOp::EQ || binExpr->op == BinaryOp::NE ||
                            binExpr->op == BinaryOp::LT || binExpr->op == BinaryOp::LE ||
                            binExpr->op == BinaryOp::GT || binExpr->op == BinaryOp::GE ||
                            binExpr->op == BinaryOp::AND || binExpr->op == BinaryOp::OR) {
                            isComparisonResult = true;
                        } else {
                            isFloatResult = isFloatExpression(binExpr->left.get()) || isFloatExpression(binExpr->right.get());
                        }
                    } else if (auto unaryExpr = dynamic_cast<UnaryExpression*>(arg.get())) {
                        // Check if it's a NOT operation (returns string)
                        if (unaryExpr->op == UnaryOp::NOT) {
                            isComparisonResult = true;
                        }
                    } else if (auto floatLit = dynamic_cast<FloatLiteral*>(arg.get())) {
                        isFloatResult = true;
                    }
                    
                    arg->accept(*this);
                    assembly << "    # Call out() with expression result\n";
                    
                    if (isComparisonResult) {
                        // Comparison results are string addresses (str_true/str_false)
                        assembly << "    mov %rax, %rsi\n";
                        assembly << "    mov $format_str, %rdi\n";  // Use string format for comparison results
                        assembly << "    xor %rax, %rax\n";
                    } else if (isFloatResult) {
                        assembly << "    movq %rax, %xmm0  # Load float result into XMM register\n";
                        assembly << "    mov $format_float, %rdi\n";
                        assembly << "    mov $1, %rax  # Number of vector registers used\n";
                    } else {
                        assembly << "    mov %rax, %rsi\n";
                        assembly << "    mov $format_int, %rdi\n";  // Use integer format for computed results
                        assembly << "    xor %rax, %rax\n";
                    }
                    assembly << "    call printf\n";
                }
            }
        } else if (node.name == "dtype") {
            // Handle standalone dtype() calls (though typically used inside out())
            if (!node.arguments.empty()) {
                auto& arg = node.arguments[0];
                if (auto id = dynamic_cast<Identifier*>(arg.get())) {
                    auto varIt = lookupVariable(id->name);
                    if (varIt != nullptr) {
                        assembly << "    # dtype(" << id->name << ") - type: " << varIt->type << "\n";
                        // For standalone dtype(), we could return a type indicator
                        // For now, just put the type string address in %rax
                        std::string dtypeLabel;
                        if (varIt->type == "int") {
                            dtypeLabel = "dtype_int";
                        } else if (varIt->type == "string") {
                            dtypeLabel = "dtype_string";
                        } else if (varIt->type == "bool") {
                            dtypeLabel = "dtype_bool";
                        } else if (varIt->type == "float") {
                            dtypeLabel = "dtype_float";
                        } else {
                            dtypeLabel = "dtype_unknown";
                        }
                        assembly << "    mov $" << dtypeLabel << ", %rax\n";
                    } else {
                        throw std::runtime_error("Line " + std::to_string(id->line) + ": Error: Undefined variable '" + id->name + "'");
                    }
                }
            }
        } else {
            // Handle user-defined function calls
            if (node.name == "main") {
                // Special handling for explicit main() calls
                assembly << "    # Explicit call to main()\n";
                executeFunctionCall("main", node.arguments);
            } else {
                // Handle other user-defined function calls
                assembly << "    # User-defined function call: " << node.name << "\n";
                executeFunctionCall(node.name, node.arguments);
            }
        }
    }
    
    void visit(BinaryExpression& node) override {
        // Check for list operations first
        if (node.op == BinaryOp::ADD) {
            // Use type inference for robust two-sided validation
            ExprKind leftKind = inferExprKind(node.left.get());
            ExprKind rightKind = inferExprKind(node.right.get());
            
            // If either operand is a list, require both to be lists
            if (leftKind == ExprKind::LIST || rightKind == ExprKind::LIST) {
                if (leftKind != ExprKind::LIST || rightKind != ExprKind::LIST) {
                    throw std::runtime_error("Error: Cannot concatenate list with non-list. Both operands of '+' must be lists.");
                }
                
                // This is list concatenation: list + list
                assembly << "    # List concatenation: list + list\n";
                
                // Evaluate left list
                node.left->accept(*this);
                assembly << "    mov %rax, %rdi  # First list as first argument\n";
                assembly << "    push %rdi  # Save first list\n";
                
                // Evaluate right list
                node.right->accept(*this);
                assembly << "    mov %rax, %rsi  # Second list as second argument\n";
                assembly << "    pop %rdi  # Restore first list\n";
                
                assembly << "    call list_concat  # Concatenate lists\n";
                return;
            }
        }
        
        if (node.op == BinaryOp::MUL) {
            // Use type inference for robust validation
            ExprKind leftKind = inferExprKind(node.left.get());
            ExprKind rightKind = inferExprKind(node.right.get());
            
            // Check for list * int or int * list (valid repetition)
            if (leftKind == ExprKind::LIST && rightKind == ExprKind::INT) {
                // This is list * n
                assembly << "    # List repetition: list * n\n";
                
                // Evaluate list
                node.left->accept(*this);
                assembly << "    mov %rax, %rdi  # List as first argument\n";
                assembly << "    push %rdi  # Save list\n";
                
                // Evaluate repeat count
                node.right->accept(*this);
                assembly << "    mov %rax, %rsi  # Repeat count as second argument\n";
                assembly << "    pop %rdi  # Restore list\n";
                
                assembly << "    call list_repeat  # Repeat list\n";
                return;
            }
            
            if (leftKind == ExprKind::INT && rightKind == ExprKind::LIST) {
                // This is n * list
                assembly << "    # List repetition: n * list\n";
                
                // Evaluate repeat count
                node.left->accept(*this);
                assembly << "    mov %rax, %rsi  # Repeat count as second argument\n";
                assembly << "    push %rsi  # Save repeat count\n";
                
                // Evaluate list
                node.right->accept(*this);
                assembly << "    mov %rax, %rdi  # List as first argument\n";
                assembly << "    pop %rsi  # Restore repeat count\n";
                
                assembly << "    call list_repeat  # Repeat list\n";
                return;
            }
            
            // Check for invalid list operations
            if (leftKind == ExprKind::LIST || rightKind == ExprKind::LIST) {
                if (leftKind == ExprKind::LIST && rightKind == ExprKind::LIST) {
                    throw std::runtime_error("Error: Cannot multiply two lists. Use + for concatenation or * with an integer for repetition.");
                } else {
                    throw std::runtime_error("Error: List repetition requires an integer. Valid operations: list * int or int * list.");
                }
            }
        }
        
        // Check if either operand is a float
        bool leftIsFloat = isFloatExpression(node.left.get());
        bool rightIsFloat = isFloatExpression(node.right.get());
        bool isFloatOperation = leftIsFloat || rightIsFloat;
        
        if (isFloatOperation) {
            // Handle floating-point arithmetic
            assembly << "    # Floating-point binary operation\n";
            
            // Evaluate left operand
            node.left->accept(*this);
            if (leftIsFloat) {
                assembly << "    movq %rax, %xmm0  # Load float left operand\n";
            } else {
                assembly << "    cvtsi2sd %rax, %xmm0  # Convert int to float (left)\n";
            }
            assembly << "    subq $8, %rsp\n";
            assembly << "    movsd %xmm0, (%rsp)  # Save left operand on stack\n";
            
            // Evaluate right operand
            node.right->accept(*this);
            if (rightIsFloat) {
                assembly << "    movq %rax, %xmm1  # Load float right operand\n";
            } else {
                assembly << "    cvtsi2sd %rax, %xmm1  # Convert int to float (right)\n";
            }
            
            // Load left operand back
            assembly << "    movsd (%rsp), %xmm0  # Restore left operand\n";
            assembly << "    addq $8, %rsp\n";
            
            // Perform floating-point operation
            switch (node.op) {
                case BinaryOp::ADD:
                    assembly << "    addsd %xmm1, %xmm0  # Float addition\n";
                    break;
                case BinaryOp::SUB:
                    assembly << "    subsd %xmm1, %xmm0  # Float subtraction\n";
                    break;
                case BinaryOp::MUL:
                    assembly << "    mulsd %xmm1, %xmm0  # Float multiplication\n";
                    break;
                case BinaryOp::DIV:
                    assembly << "    divsd %xmm1, %xmm0  # Float division\n";
                    break;
                case BinaryOp::FLOOR_DIV:
                    // Floor division: divide then apply floor function
                    assembly << "    divsd %xmm1, %xmm0  # Float division\n";
                    assembly << "    # Apply floor function\n";
                    assembly << "    subq $8, %rsp  # Align stack\n";
                    assembly << "    movsd %xmm0, (%rsp)  # Save division result\n";
                    assembly << "    movsd (%rsp), %xmm0  # Load argument for floor\n";
                    assembly << "    call floor  # Call C library floor function\n";
                    assembly << "    addq $8, %rsp  # Restore stack\n";
                    break;
                case BinaryOp::MOD:
                    // Float modulo using fmod function
                    assembly << "    # Float modulo - save registers and call fmod\n";
                    assembly << "    subq $16, %rsp  # Align stack\n";
                    assembly << "    movsd %xmm0, (%rsp)  # Save first operand\n";
                    assembly << "    movsd %xmm1, 8(%rsp)  # Save second operand\n";
                    assembly << "    movsd (%rsp), %xmm0  # Load first arg for fmod\n";
                    assembly << "    movsd 8(%rsp), %xmm1  # Load second arg for fmod\n";
                    assembly << "    call fmod  # Call C library fmod function\n";
                    assembly << "    addq $16, %rsp  # Restore stack\n";
                    break;
                case BinaryOp::POWER:
                    // Float power using pow function
                    assembly << "    # Float power - save registers and call pow\n";
                    assembly << "    subq $16, %rsp  # Align stack\n";
                    assembly << "    movsd %xmm0, (%rsp)  # Save base\n";
                    assembly << "    movsd %xmm1, 8(%rsp)  # Save exponent\n";
                    assembly << "    movsd (%rsp), %xmm0  # Load base for pow\n";
                    assembly << "    movsd 8(%rsp), %xmm1  # Load exponent for pow\n";
                    assembly << "    call pow  # Call C library pow function\n";
                    assembly << "    addq $16, %rsp  # Restore stack\n";
                    break;
                case BinaryOp::EQ:
                    assembly << "    comisd %xmm1, %xmm0\n";
                    assembly << "    je feq_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp feq_done_" << labelCounter << "\n";
                    assembly << "feq_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "feq_done_" << labelCounter << ":\n";
                    labelCounter++;
                    return;
                case BinaryOp::NE:
                    assembly << "    comisd %xmm1, %xmm0\n";
                    assembly << "    jne fne_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp fne_done_" << labelCounter << "\n";
                    assembly << "fne_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "fne_done_" << labelCounter << ":\n";
                    labelCounter++;
                    return;
                case BinaryOp::LT:
                    assembly << "    comisd %xmm1, %xmm0\n";
                    assembly << "    jb flt_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp flt_done_" << labelCounter << "\n";
                    assembly << "flt_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "flt_done_" << labelCounter << ":\n";
                    labelCounter++;
                    return;
                case BinaryOp::LE:
                    assembly << "    comisd %xmm1, %xmm0\n";
                    assembly << "    jbe fle_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp fle_done_" << labelCounter << "\n";
                    assembly << "fle_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "fle_done_" << labelCounter << ":\n";
                    labelCounter++;
                    return;
                case BinaryOp::GT:
                    assembly << "    comisd %xmm1, %xmm0\n";
                    assembly << "    ja fgt_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp fgt_done_" << labelCounter << "\n";
                    assembly << "fgt_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "fgt_done_" << labelCounter << ":\n";
                    labelCounter++;
                    return;
                case BinaryOp::GE:
                    assembly << "    comisd %xmm1, %xmm0\n";
                    assembly << "    jae fge_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp fge_done_" << labelCounter << "\n";
                    assembly << "fge_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "fge_done_" << labelCounter << ":\n";
                    labelCounter++;
                    return;
                default:
                    assembly << "    # Unsupported float operation - ERROR\n";
                    assembly << "    mov $0, %rax  # Return 0 for unsupported operations\n";
                    assembly << "    cvtsi2sd %rax, %xmm0  # Convert 0 to float\n";
                    break;
            }
            
            // Store result back to %rax as raw bits
            assembly << "    movq %xmm0, %rax  # Store float result\n";
        } else {
            // Handle integer arithmetic (original code)
            assembly << "    # Integer binary operation\n";
            
            // Evaluate left operand
            node.left->accept(*this);
            assembly << "    push %rax\n";
            
            // Evaluate right operand
            node.right->accept(*this);
            assembly << "    pop %rbx\n";
            
            // Perform operation
            switch (node.op) {
                case BinaryOp::ADD:
                    assembly << "    add %rbx, %rax\n";
                    break;
                case BinaryOp::SUB:
                    assembly << "    sub %rax, %rbx\n";
                    assembly << "    mov %rbx, %rax\n";
                    break;
                case BinaryOp::MUL:
                    assembly << "    imul %rbx, %rax\n";
                    break;
                case BinaryOp::DIV:
                    assembly << "    mov %rax, %rcx\n";
                    assembly << "    mov %rbx, %rax\n";
                    assembly << "    xor %rdx, %rdx\n";
                    assembly << "    idiv %rcx\n";
                    break;
                case BinaryOp::MOD:
                    assembly << "    mov %rax, %rcx\n";
                    assembly << "    mov %rbx, %rax\n";
                    assembly << "    xor %rdx, %rdx\n";
                    assembly << "    idiv %rcx\n";
                    assembly << "    mov %rdx, %rax\n";
                    break;
                case BinaryOp::FLOOR_DIV:
                    // Integer division (same as DIV for integers)
                    assembly << "    mov %rax, %rcx\n";
                    assembly << "    mov %rbx, %rax\n";
                    assembly << "    xor %rdx, %rdx\n";
                    assembly << "    idiv %rcx\n";
                    break;
                case BinaryOp::POWER:
                    // Simple power implementation for small integers
                    assembly << "    mov %rbx, %rcx  # base\n";
                    assembly << "    mov %rax, %rdx  # exponent\n";
                    assembly << "    mov $1, %rax    # result = 1\n";
                    assembly << "power_loop:\n";
                    assembly << "    test %rdx, %rdx\n";
                    assembly << "    jz power_done\n";
                    assembly << "    imul %rcx, %rax\n";
                    assembly << "    dec %rdx\n";
                    assembly << "    jmp power_loop\n";
                    assembly << "power_done:\n";
                    break;
                case BinaryOp::EQ:
                    assembly << "    cmp %rax, %rbx\n";
                    assembly << "    je eq_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp eq_done_" << labelCounter << "\n";
                    assembly << "eq_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "eq_done_" << labelCounter << ":\n";
                    labelCounter++;
                    break;
                case BinaryOp::NE:
                    assembly << "    cmp %rax, %rbx\n";
                    assembly << "    jne ne_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp ne_done_" << labelCounter << "\n";
                    assembly << "ne_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "ne_done_" << labelCounter << ":\n";
                    labelCounter++;
                    break;
                case BinaryOp::LT:
                    assembly << "    cmp %rax, %rbx\n";
                    assembly << "    jl lt_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp lt_done_" << labelCounter << "\n";
                    assembly << "lt_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "lt_done_" << labelCounter << ":\n";
                    labelCounter++;
                    break;
                case BinaryOp::LE:
                    assembly << "    cmp %rax, %rbx\n";
                    assembly << "    jle le_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp le_done_" << labelCounter << "\n";
                    assembly << "le_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "le_done_" << labelCounter << ":\n";
                    labelCounter++;
                    break;
                case BinaryOp::GT:
                    assembly << "    cmp %rax, %rbx\n";
                    assembly << "    jg gt_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp gt_done_" << labelCounter << "\n";
                    assembly << "gt_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "gt_done_" << labelCounter << ":\n";
                    labelCounter++;
                    break;
                case BinaryOp::GE:
                    assembly << "    cmp %rax, %rbx\n";
                    assembly << "    jge ge_true_" << labelCounter << "\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "    jmp ge_done_" << labelCounter << "\n";
                    assembly << "ge_true_" << labelCounter << ":\n";
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "ge_done_" << labelCounter << ":\n";
                    labelCounter++;
                    break;
                case BinaryOp::AND:
                    // Logical AND: both operands must be truthy
                    // Left operand is falsy if it's 0 or str_false
                    assembly << "    cmp $0, %rbx\n";
                    assembly << "    je and_false_" << labelCounter << "\n";
                    assembly << "    cmp $str_false, %rbx\n";
                    assembly << "    je and_false_" << labelCounter << "\n";
                    // Left is truthy, check right operand
                    assembly << "    cmp $0, %rax\n";
                    assembly << "    je and_false_" << labelCounter << "\n";
                    assembly << "    cmp $str_false, %rax\n";
                    assembly << "    je and_false_" << labelCounter << "\n";
                    // Both are truthy
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "    jmp and_done_" << labelCounter << "\n";
                    assembly << "and_false_" << labelCounter << ":\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "and_done_" << labelCounter << ":\n";
                    labelCounter++;
                    break;
                case BinaryOp::OR:
                    // Logical OR: either operand can be truthy
                    // Check if left operand is truthy (not 0 and not str_false)
                    assembly << "    cmp $0, %rbx\n";
                    assembly << "    je or_check_right_" << labelCounter << "\n";
                    assembly << "    cmp $str_false, %rbx\n";
                    assembly << "    je or_check_right_" << labelCounter << "\n";
                    // Left is truthy
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "    jmp or_done_" << labelCounter << "\n";
                    assembly << "or_check_right_" << labelCounter << ":\n";
                    // Left is falsy, check right operand
                    assembly << "    cmp $0, %rax\n";
                    assembly << "    je or_false_" << labelCounter << "\n";
                    assembly << "    cmp $str_false, %rax\n";
                    assembly << "    je or_false_" << labelCounter << "\n";
                    // Right is truthy
                    assembly << "    mov $str_true, %rax\n";
                    assembly << "    jmp or_done_" << labelCounter << "\n";
                    assembly << "or_false_" << labelCounter << ":\n";
                    assembly << "    mov $str_false, %rax\n";
                    assembly << "or_done_" << labelCounter << ":\n";
                    labelCounter++;
                    break;
                case BinaryOp::ASSIGN:
                    // Chain assignment: a = (b = 5)
                    // Evaluate RHS -> store to LHS -> return value in RAX
                    assembly << "    # Chain assignment\n";
                    
                    // Evaluate RHS expression
                    node.right->accept(*this);
                    // Result is now in %rax - we'll use it directly
                    
                    // Extract LHS variable name and perform assignment
                    if (auto id = dynamic_cast<Identifier*>(node.left.get())) {
                        // Check if this is an assignment to an existing const variable
                        if (constantVariables.count(id->name)) {
                            throw std::runtime_error("Error: You are trying to change the value of a constant variable '" + id->name + "'");
                        }
                        
                        // Find or create variable
                        VariableInfo* varInfo = lookupVariable(id->name);
                        if (!varInfo) {
                            // Variable doesn't exist, create it
                            stackOffset += 8;
                            VariableInfo newVarInfo;
                            newVarInfo.stackOffset = stackOffset;
                            newVarInfo.type = "unknown";
                            newVarInfo.isGlobal = (!inFunction) || declaredGlobal.count(id->name);
                            newVarInfo.isConstant = false;
                            
                            if (inFunction && !declaredGlobal.count(id->name)) {
                                localVariables[id->name] = newVarInfo;
                                varInfo = &localVariables[id->name];
                            } else {
                                globalVariables[id->name] = newVarInfo;
                                varInfo = &globalVariables[id->name];
                            }
                        }
                        
                        // Store value to variable directly from %rax
                        assembly << "    mov %rax, -" << varInfo->stackOffset << "(%rbp)  # store " << id->name << "\n";
                    } else {
                        throw std::runtime_error("Error: Left side of assignment must be a variable");
                    }
                    
                    // RAX already contains the assigned value for chaining - no need to modify it
                    break;
                default:
                    assembly << "    # Unsupported binary operation\n";
                    break;
            }
        }
    }
    
    void visit(IntLiteral& node) override {
        assembly << "    mov $" << node.value << ", %rax\n";
    }
    
    void visit(StringLiteral& node) override {
        int index = addStringLiteral(node.value);
        assembly << "    mov $str_" << index << ", %rax\n";
    }
    
    void visit(Identifier& node) override {
        // Python-style variable lookup: local scope first, then global scope
        VariableInfo* varInfo = lookupVariable(node.name);
        if (varInfo != nullptr) {
            assembly << "    mov -" << varInfo->stackOffset << "(%rbp), %rax  # load " << (varInfo->isGlobal ? "global" : "local") << " " << node.name << "\n";
        } else {
            std::string errorMsg = "Error: Undefined variable '" + node.name + "'";
            if (node.line > 0) {
                errorMsg = "Line " + std::to_string(node.line) + ": " + errorMsg;
            }
            throw std::runtime_error(errorMsg);
        }
    }
    
    void visit(TupleExpression& node) override {
        // For now, tuples just evaluate to the last element
        // In a full implementation, this would need proper tuple handling
        if (!node.elements.empty()) {
            node.elements.back()->accept(*this);
        }
    }
    
    void visit(TupleAssignment& node) override {
        // Handle tuple assignment like (a, b) = (c, d)
        // Algorithm: evaluate all RHS values first into stack temporaries,
        // then assign to LHS variables to prevent overwriting during swaps
        assembly << "    # Tuple assignment\n";
        
        if (node.targets.size() != node.values.size()) {
            throw std::runtime_error("Error: Tuple assignment mismatch - number of targets (" + 
                std::to_string(node.targets.size()) + ") doesn't match number of values (" + 
                std::to_string(node.values.size()) + ")");
        }
        
        // Check if any target variables are const before doing anything
        for (const auto& target : node.targets) {
            if (auto id = dynamic_cast<Identifier*>(target.get())) {
                if (constantVariables.count(id->name)) {
                    throw std::runtime_error("Error: You are trying to change the value of a constant variable '" + id->name + "'");
                }
            }
        }
        
        // Step 1: Evaluate all RHS values and push them onto the stack
        assembly << "    # Step 1: Evaluate all RHS values\n";
        for (size_t i = 0; i < node.values.size(); i++) {
            assembly << "    # Evaluating RHS value " << i << "\n";
            node.values[i]->accept(*this);
            assembly << "    push %rax  # Save RHS value " << i << " on stack\n";
        }
        
        // Step 2: Assign to LHS variables in reverse order (since stack is LIFO)
        assembly << "    # Step 2: Assign to LHS variables\n";
        for (int i = node.targets.size() - 1; i >= 0; i--) {
            assembly << "    # Assigning to LHS target " << i << "\n";
            assembly << "    pop %rax  # Get value " << i << " from stack\n";
            
            if (auto id = dynamic_cast<Identifier*>(node.targets[i].get())) {
                // Find or create variable
                VariableInfo* varInfo = lookupVariable(id->name);
                if (!varInfo) {
                    // Variable doesn't exist, create it
                    stackOffset += 8;
                    VariableInfo newVarInfo;
                    newVarInfo.stackOffset = stackOffset;
                    newVarInfo.type = "unknown";
                    newVarInfo.isGlobal = (!inFunction) || declaredGlobal.count(id->name);
                    newVarInfo.isConstant = false;
                    
                    if (inFunction && !declaredGlobal.count(id->name)) {
                        localVariables[id->name] = newVarInfo;
                        varInfo = &localVariables[id->name];
                    } else {
                        globalVariables[id->name] = newVarInfo;
                        varInfo = &globalVariables[id->name];
                    }
                }
                
                // Store the value
                assembly << "    mov %rax, -" << varInfo->stackOffset << "(%rbp)  # store " << id->name << "\n";
            } else {
                throw std::runtime_error("Error: Left side of tuple assignment must be variables");
            }
        }
        
        assembly << "    # Tuple assignment complete\n";
    }

    void visit(ChainAssignment& node) override {
        assembly << "    # Chain assignment\n";
        
        // Check if any variables are const
        for (const std::string& varName : node.variables) {
            if (constantVariables.count(varName)) {
                throw std::runtime_error("Error: You are trying to change the value of a constant variable '" + varName + "'");
            }
        }
        
        // Evaluate the value expression once
        node.value->accept(*this);
        // Value is now in %rax - no need to push/pop, just assign directly to each variable
        
        // Determine variable type from the value expression
        std::string varType = "unknown";
        if (auto intLit = dynamic_cast<IntLiteral*>(node.value.get())) {
            varType = "int";
        } else if (auto strLit = dynamic_cast<StringLiteral*>(node.value.get())) {
            varType = "string";
        } else if (auto boolLit = dynamic_cast<BoolLiteral*>(node.value.get())) {
            varType = "bool";
        } else if (auto floatLit = dynamic_cast<FloatLiteral*>(node.value.get())) {
            varType = "float";
        } else if (auto listLit = dynamic_cast<ListLiteral*>(node.value.get())) {
            varType = "list";
        } else if (auto id = dynamic_cast<Identifier*>(node.value.get())) {
            // Variable assignment: copy type from source variable
            auto varInfo = lookupVariable(id->name);
            if (varInfo != nullptr) {
                varType = varInfo->type;
            }
        } else if (auto binExpr = dynamic_cast<BinaryExpression*>(node.value.get())) {
            // Binary expression: infer type from operands
            switch (binExpr->op) {
                case BinaryOp::ADD:
                case BinaryOp::SUB:
                case BinaryOp::MUL:
                case BinaryOp::DIV:
                case BinaryOp::MOD:
                case BinaryOp::FLOOR_DIV:
                case BinaryOp::POWER:
                    // Check if either operand is a float
                    if (isFloatExpression(binExpr->left.get()) || isFloatExpression(binExpr->right.get())) {
                        varType = "float";
                    } else {
                        varType = "int";
                    }
                    break;
                case BinaryOp::EQ:
                case BinaryOp::NE:
                case BinaryOp::LT:
                case BinaryOp::LE:
                case BinaryOp::GT:
                case BinaryOp::GE:
                case BinaryOp::AND:
                case BinaryOp::OR:
                    varType = "bool";  // Comparison/logical operations result in bool
                    break;
                default:
                    varType = "int";  // Default to int for unknown operations
                    break;
            }
        }
        
        // Assign the same value to ALL variables in the chain
        for (const std::string& varName : node.variables) {
            // Find or create variable
            VariableInfo* varInfo = lookupVariable(varName);
            if (!varInfo) {
                // Variable doesn't exist, create it
                stackOffset += 8;
                VariableInfo newVarInfo;
                newVarInfo.stackOffset = stackOffset;
                newVarInfo.type = varType;  // Use inferred type
                newVarInfo.isGlobal = (!inFunction) || declaredGlobal.count(varName);
                newVarInfo.isConstant = false;
                
                if (inFunction && !declaredGlobal.count(varName)) {
                    localVariables[varName] = newVarInfo;
                    varInfo = &localVariables[varName];
                } else {
                    globalVariables[varName] = newVarInfo;
                    varInfo = &globalVariables[varName];
                }
            } else {
                // Variable exists, update its type
                varInfo->type = varType;
            }
            
            // Store the value directly from %rax to this variable
            assembly << "    mov %rax, -" << varInfo->stackOffset << "(%rbp)  # assign to " << varName << "\n";
        }
        
        // Value remains in %rax for potential nested assignments or return
    }

    void visit(IndexAssignment& node) override {
        assembly << "    # Index assignment: list[index] = value\n";
        
        // Evaluate the list expression
        node.object->accept(*this);
        assembly << "    mov %rax, %r12  # Save list pointer in %r12\n";
        
        // Evaluate the index expression
        node.index->accept(*this);
        assembly << "    mov %rax, %r13  # Save index in %r13\n";
        
        // Evaluate the value expression  
        node.value->accept(*this);
        assembly << "    mov %rax, %rdx  # Value in %rdx (third argument)\n";
        
        // Call list_set(list, index, value)
        assembly << "    mov %r12, %rdi  # List pointer as first argument\n";
        assembly << "    mov %r13, %rsi  # Index as second argument\n";
        assembly << "    # Value already in %rdx as third argument\n";
        assembly << "    call list_set  # Set list[index] = value\n";
    }

    // Stub implementations for other visitors
    void visit(FloatLiteral& node) override { 
        assembly << "    # Float: " << node.value << "\n"; 
        // Store float value as IEEE 754 in memory and load to rax
        int floatIndex = addFloatLiteral(node.value);
        assembly << "    movq float_" << floatIndex << "(%rip), %rax\n";
    }
    void visit(BoolLiteral& node) override { 
        assembly << "    mov $" << (node.value ? "str_true" : "str_false") << ", %rax\n";
    }
    void visit(UnaryExpression& node) override {
        switch (node.op) {
            case UnaryOp::NOT:
                // Logical NOT: flip boolean result
                node.operand->accept(*this);
                // Check if operand result is falsy (0 or str_false)
                assembly << "    cmp $0, %rax\n";
                assembly << "    je not_true_" << labelCounter << "\n";
                assembly << "    cmp $str_false, %rax\n";
                assembly << "    je not_true_" << labelCounter << "\n";
                // Operand is truthy, so NOT result is false
                assembly << "    mov $str_false, %rax\n";
                assembly << "    jmp not_done_" << labelCounter << "\n";
                assembly << "not_true_" << labelCounter << ":\n";
                // Operand is falsy, so NOT result is true
                assembly << "    mov $str_true, %rax\n";
                assembly << "not_done_" << labelCounter << ":\n";
                labelCounter++;
                break;
            case UnaryOp::PLUS:
                // Unary plus - just evaluate operand
                node.operand->accept(*this);
                break;
            case UnaryOp::MINUS:
                // Unary minus - negate the operand
                node.operand->accept(*this);
                assembly << "    neg %rax\n";
                break;
        }
    }
    void visit(BlockStatement& node) override { 
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
    }
    void visit(ReturnStatement& node) override { 
        if (node.value) node.value->accept(*this);
    }
    void visit(IfStatement& node) override { }
    void visit(WhileStatement& node) override { }
    void visit(ForStatement& node) override { }
    void visit(GlobalStatement& node) override {
        for (const std::string& varName : node.variables) {
            declaredGlobal.insert(varName);
            assembly << "    # Global declaration: " << varName << "\n";
        }
    }
    
    void visit(LocalStatement& node) override {
        for (const std::string& varName : node.variables) {
            declaredLocal.insert(varName);
            assembly << "    # Local declaration: " << varName << "\n";
        }
    }
    
    void visit(ListLiteral& node) override {
        assembly << "    # Enhanced list literal with " << node.elements.size() << " elements\n";
        
        if (node.elements.empty()) {
            // Create empty list using runtime
            assembly << "    mov $4, %rdi  # Initial capacity for empty list\n";
            assembly << "    call list_new  # Create new empty list\n";
            return;
        }
        
        // For non-empty lists, collect elements in temporary array first
        assembly << "    # Allocating temporary array for " << node.elements.size() << " elements\n";
        size_t tempArraySize = node.elements.size() * 8;  // 8 bytes per element
        assembly << "    mov $" << tempArraySize << ", %rdi\n";
        assembly << "    call orion_malloc  # Allocate temporary array\n";
        assembly << "    mov %rax, %r12  # Save temp array pointer in %r12\n";
        
        // Store each element in temporary array
        for (size_t i = 0; i < node.elements.size(); i++) {
            assembly << "    # Evaluating element " << i << "\n";
            assembly << "    push %r12  # Save temp array pointer\n";
            node.elements[i]->accept(*this);  // Element value in %rax
            assembly << "    pop %r12  # Restore temp array pointer\n";
            assembly << "    movq %rax, " << (i * 8) << "(%r12)  # Store in temp array\n";
        }
        
        // Create list from temporary data
        assembly << "    mov %r12, %rdi  # Temp array pointer\n";
        assembly << "    mov $" << node.elements.size() << ", %rsi  # Element count\n";
        assembly << "    call list_from_data  # Create list from data\n";
        
        // Free temporary array - list_from_data made a copy
        assembly << "    push %rax  # Save list pointer\n";
        assembly << "    mov %r12, %rdi  # Temp array pointer\n";
        assembly << "    call orion_free  # Free temporary array\n";
        assembly << "    pop %rax  # Restore list pointer\n";
    }
    
    void visit(IndexExpression& node) override {
        assembly << "    # Enhanced index expression with negative indexing support\n";
        
        // Evaluate the object (list) - result in %rax
        node.object->accept(*this);
        assembly << "    mov %rax, %rdi  # List pointer as first argument\n";
        
        // Evaluate the index - result in %rax
        node.index->accept(*this);
        assembly << "    mov %rax, %rsi  # Index as second argument\n";
        
        // Call runtime function for safe indexing with negative support
        assembly << "    call list_get  # Get element with bounds checking\n";
        // Result is in %rax - no additional handling needed
    }
    
    void visit(StructDeclaration& node) override { }
    void visit(EnumDeclaration& node) override { }
};


} // namespace orion

// Compiler main function
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source-file>" << std::endl;
        return 1;
    }
    
    std::string filename = argv[1];
    
    try {
        // Read source file
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return 1;
        }
        
        std::string source((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
        file.close();
        
        // Split source into lines for error reporting
        std::vector<std::string> sourceLines;
        std::stringstream ss(source);
        std::string line;
        while (std::getline(ss, line)) {
            sourceLines.push_back(line);
        }
        
        // Step 1: Lexical analysis
        orion::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        // Step 2: Parsing
        orion::SimpleOrionParser parser(tokens);
        auto ast = parser.parse();
        
        // Note: Type checking would be done here for better error messages
        // but we'll focus on runtime error improvements for now
        
        // Step 3: Code generation
        orion::SimpleCodeGenerator codegen;
        std::string assembly = codegen.generate(*ast);
        
        // Step 4: Write assembly to file (KEEP FOR PROOF)
        std::string asmFile = "orion_asm.s";
        std::ofstream asmOut(asmFile);
        asmOut << assembly;
        asmOut.close();
        
        // Step 5: Use GCC to assemble and link with runtime (KEEP EXECUTABLE FOR PROOF)
        std::string exeFile = "orion_exec";
        std::string gccCommand = "gcc -o " + exeFile + " " + asmFile + " runtime.o -lm";
        
        int result = system(gccCommand.c_str());
        if (result != 0) {
            std::cerr << "Error: Failed to assemble program" << std::endl;
            return 1;
        }
        
        // Step 6: Execute the compiled program
        result = system(("./" + exeFile).c_str());
        
        // DON'T clean up - leave files for proof
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}