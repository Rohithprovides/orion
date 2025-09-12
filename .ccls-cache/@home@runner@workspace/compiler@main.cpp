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
    std::unordered_map<std::string, FunctionDeclaration*> functions; // Store function definitions
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
        fullAssembly << ".extern malloc\n";
        fullAssembly << ".extern exit\n";
        fullAssembly << ".extern fmod\n";
        fullAssembly << ".extern pow\n\n";
        
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
        // First pass: collect all function definitions (including nested ones)
        collectFunctions(node.statements);
        
        // Second pass: execute only non-function statements and function calls
        for (auto& stmt : node.statements) {
            if (dynamic_cast<FunctionDeclaration*>(stmt.get()) == nullptr) {
                stmt->accept(*this);
            }
        }
        
        // Third pass: if a user-defined main() function exists, call it
        if (findFunction("main") != nullptr) {
            assembly << "    # Calling user-defined main() function\n";
            std::vector<std::unique_ptr<Expression>> emptyArgs;
            executeFunctionCall("main", emptyArgs);
        }
    }
    
    void collectFunctions(const std::vector<std::unique_ptr<Statement>>& statements) {
        for (auto& stmt : statements) {
            if (auto func = dynamic_cast<FunctionDeclaration*>(stmt.get())) {
                // Store function definition
                functions[func->name] = func;
                assembly << "    # Function defined: " << func->name << "\n";
                
                // Recursively collect nested functions from this function's body
                if (!func->isSingleExpression) {
                    collectFunctions(func->body);
                }
            } else if (auto block = dynamic_cast<BlockStatement*>(stmt.get())) {
                // Recursively collect nested functions
                collectFunctions(block->statements);
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
            throw std::runtime_error("Error: Undefined function '" + functionName + "'");
        }
        
        // Save current function state
        bool wasInFunction = inFunction;
        auto savedLocalVars = localVariables;
        auto savedDeclaredGlobal = declaredGlobal;
        auto savedDeclaredLocal = declaredLocal;
        
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
        
        // Exit function scope - restore previous state
        inFunction = wasInFunction;
        localVariables = savedLocalVars; // Restore previous local scope
        declaredGlobal = savedDeclaredGlobal;
        declaredLocal = savedDeclaredLocal;
    }
    
    FunctionDeclaration* findFunction(const std::string& name) {
        auto it = functions.find(name);
        if (it != functions.end()) {
            return it->second;
        }
        return nullptr;
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
        // Simplified implementation for now
        assembly << "    # Tuple assignment (simplified)\n";
        // TODO: Implement proper tuple assignment with new scoping
    }

    void visit(ChainAssignment& node) override {
        assembly << "    # Chain assignment (simplified)\n";
        
        // Check if any variables are const
        for (const std::string& varName : node.variables) {
            if (constantVariables.count(varName)) {
                throw std::runtime_error("Error: You are trying to change the value of a constant variable '" + varName + "'");
            }
        }
        
        // Evaluate the value expression
        node.value->accept(*this);
        
        // TODO: Implement proper chain assignment with new scoping
        // For now, just handle as simple assignment to first variable
        if (!node.variables.empty()) {
            const std::string& varName = node.variables[0];
            // Create variable using new scoping system
            stackOffset += 8;
            VariableInfo varInfo;
            varInfo.stackOffset = stackOffset;
            varInfo.type = "unknown";
            varInfo.isGlobal = !inFunction;
            varInfo.isConstant = false;
            
            if (inFunction && !declaredGlobal.count(varName)) {
                localVariables[varName] = varInfo;
            } else {
                globalVariables[varName] = varInfo;
            }
            assembly << "    mov %rax, -" << stackOffset << "(%rbp)  # assign to " << varName << "\n";
        }
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
        assembly << "    # List literal with " << node.elements.size() << " elements\n";
        
        if (node.elements.empty()) {
            // Empty list - allocate memory with size 0 for consistency
            assembly << "    mov $8, %rdi  # Allocation size for empty list (just size header)\n";
            assembly << "    call malloc  # Allocate memory for empty list\n";
            assembly << "    mov %rax, %rbx  # Save list pointer\n";
            assembly << "    movq $0, (%rbx)  # Store list size = 0\n";
            assembly << "    mov %rbx, %rax  # Return list pointer\n";
            return;
        }
        
        // Allocate memory: size = 8 bytes (size) + 8 * element_count bytes (elements)
        size_t totalSize = 8 + (8 * node.elements.size());
        assembly << "    mov $" << totalSize << ", %rdi  # Allocation size\n";
        assembly << "    call malloc  # Allocate memory for list\n";
        assembly << "    mov %rax, %rbx  # Save list pointer\n";
        
        // Store list size at the beginning
        assembly << "    movq $" << node.elements.size() << ", (%rbx)  # Store list size\n";
        
        // Store each element - preserve %rbx across evaluations
        for (size_t i = 0; i < node.elements.size(); i++) {
            assembly << "    # Store element " << i << "\n";
            assembly << "    push %rbx  # Save list pointer\n";
            node.elements[i]->accept(*this);  // Element value in %rax
            assembly << "    pop %rbx  # Restore list pointer\n";
            assembly << "    movq %rax, " << (8 + i * 8) << "(%rbx)  # Store element " << i << "\n";
        }
        
        // Return list pointer in %rax
        assembly << "    mov %rbx, %rax  # List pointer\n";
    }
    
    void visit(IndexExpression& node) override {
        assembly << "    # Index expression: array[index]\n";
        
        // Evaluate the object (list) - result in %rax
        node.object->accept(*this);
        assembly << "    mov %rax, %rbx  # Save list pointer\n";
        
        // Check for null list
        assembly << "    test %rbx, %rbx\n";
        assembly << "    jz index_error_" << labelCounter << "  # Jump if null list\n";
        
        // Evaluate the index - result in %rax
        node.index->accept(*this);
        assembly << "    mov %rax, %rcx  # Save index\n";
        
        // Bounds checking: get list size
        assembly << "    movq (%rbx), %rdx  # Load list size\n";
        assembly << "    cmp %rdx, %rcx\n";
        assembly << "    jge index_error_" << labelCounter << "  # Jump if index >= size\n";
        assembly << "    test %rcx, %rcx\n";
        assembly << "    js index_error_" << labelCounter << "  # Jump if index < 0\n";
        
        // Calculate element address: base + 8 + (index * 8)
        assembly << "    imul $8, %rcx  # index * 8\n";
        assembly << "    add $8, %rcx  # Add header offset\n";
        assembly << "    add %rbx, %rcx  # base + offset\n";
        assembly << "    movq (%rcx), %rax  # Load element value\n";
        assembly << "    jmp index_done_" << labelCounter << "\n";
        
        // Error handling
        assembly << "index_error_" << labelCounter << ":\n";
        assembly << "    # Print index error message and terminate\n";
        assembly << "    mov $str_index_error, %rsi\n";
        assembly << "    mov $format_str, %rdi\n";
        assembly << "    xor %rax, %rax\n";
        assembly << "    call printf\n";
        assembly << "    mov $1, %rdi  # Exit code 1 for error\n";
        assembly << "    call exit  # Terminate program\n";
        
        assembly << "index_done_" << labelCounter << ":\n";
        labelCounter++;
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
        
        // Step 5: Use GCC to assemble and link (KEEP EXECUTABLE FOR PROOF)
        std::string exeFile = "orion_exec";
        std::string gccCommand = "gcc -o " + exeFile + " " + asmFile + " -lm";
        
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