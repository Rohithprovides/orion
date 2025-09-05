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
    struct VariableInfo {
        int stackOffset;
        std::string type;
        bool isGlobal;
    };
    std::unordered_map<std::string, VariableInfo> globalVariables; // Global scope variables
    std::unordered_map<std::string, VariableInfo> localVariables; // Current function scope variables
    std::unordered_set<std::string> declaredGlobal; // Variables explicitly declared global with 'global' keyword
    std::unordered_set<std::string> declaredLocal;  // Variables explicitly declared local with 'local' keyword
    std::unordered_map<std::string, FunctionDeclaration*> functions; // Store function definitions
    int stackOffset = 0;
    bool inFunction = false;
    int labelCounter = 0;
    
    std::string newLabel(const std::string& prefix = "L") {
        return prefix + std::to_string(labelCounter++);
    }
    
    int addStringLiteral(const std::string& str) {
        stringLiterals.push_back(str);
        return stringLiterals.size() - 1;
    }
    
public:
    std::string generate(Program& program) {
        assembly.str("");
        assembly.clear();
        stringLiterals.clear();
        globalVariables.clear();
        localVariables.clear();
        declaredGlobal.clear();
        declaredLocal.clear();
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
        fullAssembly << "format_str: .string \"%s\\n\"\n";
        fullAssembly << "dtype_int: .string \"datatype: int\\n\"\n";
        fullAssembly << "dtype_string: .string \"datatype: string\\n\"\n";
        fullAssembly << "dtype_bool: .string \"datatype: bool\\n\"\n";
        fullAssembly << "dtype_float: .string \"datatype: float\\n\"\n";
        fullAssembly << "dtype_unknown: .string \"datatype: unknown\\n\"\n";
        
        // String literals
        for (size_t i = 0; i < stringLiterals.size(); i++) {
            fullAssembly << "str_" << i << ": .string \"" << stringLiterals[i] << "\\n\"\n";
        }
        
        // Text section
        fullAssembly << "\n.section .text\n";
        fullAssembly << ".global main\n";
        fullAssembly << ".extern printf\n\n";
        
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
            } else if (auto id = dynamic_cast<Identifier*>(node.initializer.get())) {
                // Variable assignment: copy type from source variable
                auto varInfo = lookupVariable(id->name);
                if (varInfo != nullptr) {
                    varType = varInfo->type;
                }
            }
            
            node.initializer->accept(*this);
            
            // Python-style scoping rules
            if (declaredGlobal.count(node.name) || (!inFunction)) {
                // Explicitly declared global OR not in function - use global scope
                stackOffset += 8;
                VariableInfo varInfo;
                varInfo.stackOffset = stackOffset;
                varInfo.type = varType;
                varInfo.isGlobal = true;
                globalVariables[node.name] = varInfo;
                assembly << "    mov %rax, -" << stackOffset << "(%rbp)  # store global " << node.name << "\n";
            } else {
                // In function and not declared global - create local variable
                stackOffset += 8;
                VariableInfo varInfo;
                varInfo.stackOffset = stackOffset;
                varInfo.type = varType;
                varInfo.isGlobal = false;
                localVariables[node.name] = varInfo;
                assembly << "    mov %rax, -" << stackOffset << "(%rbp)  # store local " << node.name << "\n";
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
                        
                        if (it->type == "int" || it->type == "bool") {
                            assembly << "    mov $format_int, %rdi\n";
                        } else {
                            assembly << "    mov $format_str, %rdi\n";
                        }
                        
                        assembly << "    xor %rax, %rax\n";
                        assembly << "    call printf\n";
                    } else {
                        throw std::runtime_error("Error: Undefined variable '" + id->name + "'");
                    }
                } else {
                    // Generic expression
                    arg->accept(*this);
                    assembly << "    # Call out() with expression result\n";
                    assembly << "    mov %rax, %rsi\n";
                    assembly << "    mov $format_str, %rdi\n";
                    assembly << "    xor %rax, %rax\n";
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
        // Handle string concatenation and other operations
        if (node.op == BinaryOp::ADD) {
            // For now, simple integer addition
            node.left->accept(*this);
            assembly << "    push %rax\n";
            node.right->accept(*this);
            assembly << "    pop %rbx\n";
            assembly << "    add %rbx, %rax\n";
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
            
            if (inFunction && !declaredGlobal.count(varName)) {
                localVariables[varName] = varInfo;
            } else {
                globalVariables[varName] = varInfo;
            }
            assembly << "    mov %rax, -" << stackOffset << "(%rbp)  # assign to " << varName << "\n";
        }
    }

    // Stub implementations for other visitors
    void visit(FloatLiteral& node) override { assembly << "    # Float: " << node.value << "\n"; }
    void visit(BoolLiteral& node) override { assembly << "    mov $" << (node.value ? 1 : 0) << ", %rax\n"; }
    void visit(UnaryExpression& node) override { node.operand->accept(*this); }
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
        std::string asmFile = "proof_assembly.s";
        std::ofstream asmOut(asmFile);
        asmOut << assembly;
        asmOut.close();
        
        // Step 5: Use GCC to assemble and link (KEEP EXECUTABLE FOR PROOF)
        std::string exeFile = "proof_executable";
        std::string gccCommand = "gcc -o " + exeFile + " " + asmFile;
        
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