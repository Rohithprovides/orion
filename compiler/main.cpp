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
    std::unordered_map<std::string, VariableInfo> variables; // All variables
    std::unordered_set<std::string> globalVars; // Variables declared global
    std::unordered_set<std::string> localVars;  // Variables declared local  
    std::unordered_map<std::string, VariableInfo> shadowedVars; // Variables shadowed by locals
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
        variables.clear();
        globalVars.clear();
        localVars.clear();
        shadowedVars.clear();
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
        
        // Enter function scope
        inFunction = true;
        shadowedVars.clear(); // Clear shadowed variables from previous functions
        
        // Execute function body
        if (func->isSingleExpression) {
            func->expression->accept(*this);
        } else {
            for (auto& stmt : func->body) {
                stmt->accept(*this);
            }
        }
        
        // Exit function scope - restore shadowed variables
        for (const auto& pair : shadowedVars) {
            variables[pair.first] = pair.second; // Restore shadowed variable
        }
        shadowedVars.clear();
        inFunction = false;
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
                auto varIt = variables.find(id->name);
                if (varIt != variables.end()) {
                    varType = varIt->second.type;
                }
            }
            
            node.initializer->accept(*this);
            
            // Python-style scoping rules
            if (globalVars.count(node.name)) {
                // Explicitly declared global - always use/update global variable
                auto it = variables.find(node.name);
                if (it != variables.end()) {
                    // Update existing global variable
                    assembly << "    mov %rax, -" << it->second.stackOffset << "(%rbp)\n";
                    it->second.type = varType;
                } else {
                    // Create new global variable
                    stackOffset += 8;
                    VariableInfo varInfo;
                    varInfo.stackOffset = stackOffset;
                    varInfo.type = varType;
                    varInfo.isGlobal = true;
                    variables[node.name] = varInfo;
                    assembly << "    mov %rax, -" << stackOffset << "(%rbp)\n";
                }
            } else if (inFunction && !globalVars.count(node.name)) {
                // In function and not declared global - create/update local variable
                auto it = variables.find(node.name);
                if (it != variables.end() && !it->second.isGlobal) {
                    // Update existing local variable
                    assembly << "    mov %rax, -" << it->second.stackOffset << "(%rbp)\n";
                    it->second.type = varType;
                } else {
                    // Create new local variable (shadow global if it exists)
                    if (it != variables.end() && it->second.isGlobal) {
                        // Shadow the global variable
                        shadowedVars[node.name] = it->second;
                    }
                    stackOffset += 8;
                    VariableInfo varInfo;
                    varInfo.stackOffset = stackOffset;
                    varInfo.type = varType;
                    varInfo.isGlobal = false;
                    variables[node.name] = varInfo;
                    assembly << "    mov %rax, -" << stackOffset << "(%rbp)\n";
                }
            } else {
                // Not in function - create/update global variable
                auto it = variables.find(node.name);
                if (it != variables.end()) {
                    // Update existing global variable
                    assembly << "    mov %rax, -" << it->second.stackOffset << "(%rbp)\n";
                    it->second.type = varType;
                } else {
                    // Create new global variable
                    stackOffset += 8;
                    VariableInfo varInfo;
                    varInfo.stackOffset = stackOffset;
                    varInfo.type = varType;
                    varInfo.isGlobal = true;
                    variables[node.name] = varInfo;
                    assembly << "    mov %rax, -" << stackOffset << "(%rbp)\n";
                }
            }
        }
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
                            auto varIt = variables.find(id->name);
                            if (varIt != variables.end()) {
                                assembly << "    # Call out(dtype(" << id->name << "))\n";
                                std::string dtypeLabel;
                                if (varIt->second.type == "int") {
                                    dtypeLabel = "dtype_int";
                                } else if (varIt->second.type == "string") {
                                    dtypeLabel = "dtype_string";
                                } else if (varIt->second.type == "bool") {
                                    dtypeLabel = "dtype_bool";
                                } else if (varIt->second.type == "float") {
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
                    auto it = variables.find(id->name);
                    if (it != variables.end()) {
                        assembly << "    # Call out() with variable: " << id->name << " (type: " << it->second.type << ")\n";
                        assembly << "    mov -" << it->second.stackOffset << "(%rbp), %rsi\n";
                        
                        if (it->second.type == "int" || it->second.type == "bool") {
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
                    auto varIt = variables.find(id->name);
                    if (varIt != variables.end()) {
                        assembly << "    # dtype(" << id->name << ") - type: " << varIt->second.type << "\n";
                        // For standalone dtype(), we could return a type indicator
                        // For now, just put the type string address in %rax
                        std::string dtypeLabel;
                        if (varIt->second.type == "int") {
                            dtypeLabel = "dtype_int";
                        } else if (varIt->second.type == "string") {
                            dtypeLabel = "dtype_string";
                        } else if (varIt->second.type == "bool") {
                            dtypeLabel = "dtype_bool";
                        } else if (varIt->second.type == "float") {
                            dtypeLabel = "dtype_float";
                        } else {
                            dtypeLabel = "dtype_unknown";
                        }
                        assembly << "    mov $" << dtypeLabel << ", %rax\n";
                    } else {
                        throw std::runtime_error("Line " + std::to_string(id->line) + ": Error: Undefined variable '" + node.name + "'");
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
        // Simple lookup in the variables map
        auto it = variables.find(node.name);
        if (it != variables.end()) {
            assembly << "    mov -" << it->second.stackOffset << "(%rbp), %rax\n";
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
        // We need to evaluate all values first, then assign them
        
        if (node.targets.size() != node.values.size()) {
            throw std::runtime_error("Tuple assignment size mismatch");
        }
        
        assembly << "    # Tuple assignment\n";
        
        // Store all values in temporary registers/stack locations
        std::vector<int> tempOffsets;
        for (size_t i = 0; i < node.values.size(); i++) {
            node.values[i]->accept(*this);
            stackOffset += 8;
            tempOffsets.push_back(stackOffset);
            assembly << "    mov %rax, -" << stackOffset << "(%rbp)  # temp " << i << "\n";
        }
        
        // Now assign the temporary values to targets
        for (size_t i = 0; i < node.targets.size(); i++) {
            if (auto id = dynamic_cast<Identifier*>(node.targets[i].get())) {
                auto it = variables.find(id->name);
                if (it != variables.end()) {
                    assembly << "    mov -" << tempOffsets[i] << "(%rbp), %rax  # load temp " << i << "\n";
                    assembly << "    mov %rax, -" << it->second.stackOffset << "(%rbp)  # assign to " << id->name << "\n";
                }
            }
        }
    }

    void visit(ChainAssignment& node) override {
        assembly << "    # Chain assignment\n";
        
        // Evaluate the value expression
        node.value->accept(*this);
        
        // Assign to all variables in the chain
        for (const auto& varName : node.variables) {
            // Check if variable already exists, if not create it
            auto it = variables.find(varName);
            if (it == variables.end()) {
                // Create new variable
                stackOffset += 8;
                VariableInfo varInfo;
                varInfo.stackOffset = stackOffset;
                varInfo.isGlobal = false;
                
                // Determine type from the value expression
                if (auto intLit = dynamic_cast<IntLiteral*>(node.value.get())) {
                    varInfo.type = "int";
                } else if (auto strLit = dynamic_cast<StringLiteral*>(node.value.get())) {
                    varInfo.type = "string";
                } else if (auto boolLit = dynamic_cast<BoolLiteral*>(node.value.get())) {
                    varInfo.type = "bool";
                } else if (auto floatLit = dynamic_cast<FloatLiteral*>(node.value.get())) {
                    varInfo.type = "float";
                } else {
                    varInfo.type = "unknown";
                }
                variables[varName] = varInfo;
                assembly << "    mov %rax, -" << stackOffset << "(%rbp)  # assign to " << varName << "\n";
            } else {
                // Variable exists, update its type
                if (auto intLit = dynamic_cast<IntLiteral*>(node.value.get())) {
                    it->second.type = "int";
                } else if (auto strLit = dynamic_cast<StringLiteral*>(node.value.get())) {
                    it->second.type = "string";
                } else if (auto boolLit = dynamic_cast<BoolLiteral*>(node.value.get())) {
                    it->second.type = "bool";
                } else if (auto floatLit = dynamic_cast<FloatLiteral*>(node.value.get())) {
                    it->second.type = "float";
                } else {
                    it->second.type = "unknown";
                }
                assembly << "    mov %rax, -" << it->second.stackOffset << "(%rbp)  # assign to " << varName << "\n";
            }
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
            globalVars.insert(varName);
            assembly << "    # Global declaration: " << varName << "\n";
        }
    }
    
    void visit(LocalStatement& node) override {
        for (const std::string& varName : node.variables) {
            localVars.insert(varName);
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