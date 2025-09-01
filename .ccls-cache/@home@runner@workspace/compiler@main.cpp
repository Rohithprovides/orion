#include "ast.h"
#include "lexer.h"
#include "simple_parser.h"
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

namespace orion {


// Simplified Code Generator for basic functionality
class SimpleCodeGenerator : public ASTVisitor {
private:
    std::ostringstream assembly;
    std::vector<std::string> stringLiterals;
    std::unordered_map<std::string, int> variables; // Variable name -> stack offset
    std::unordered_map<std::string, std::string> variableTypes; // Variable name -> type
    int stackOffset = 0;
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
        variableTypes.clear();
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
        
        // Program code
        fullAssembly << assembly.str();
        
        // Return 0
        fullAssembly << "    mov $0, %rax\n";
        fullAssembly << "    pop %rbp\n";
        fullAssembly << "    ret\n";
        
        return fullAssembly.str();
    }
    
    void visit(Program& node) override {
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
    }
    
    void visit(FunctionDeclaration& node) override {
        if (node.name == "main") {
            // Generate main function body
            if (node.isSingleExpression) {
                node.expression->accept(*this);
            } else {
                for (auto& stmt : node.body) {
                    stmt->accept(*this);
                }
            }
        }
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
            }
            
            node.initializer->accept(*this);
            
            // Store variable on stack and track its type
            stackOffset += 8;
            variables[node.name] = stackOffset;
            variableTypes[node.name] = varType;
            assembly << "    mov %rax, -" << stackOffset << "(%rbp)\n";
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
                    auto typeIt = variableTypes.find(id->name);
                    if (it != variables.end() && typeIt != variableTypes.end()) {
                        assembly << "    # Call out() with variable: " << id->name << " (type: " << typeIt->second << ")\n";
                        assembly << "    mov -" << it->second << "(%rbp), %rsi\n";
                        
                        if (typeIt->second == "int" || typeIt->second == "bool") {
                            assembly << "    mov $format_int, %rdi\n";
                        } else {
                            assembly << "    mov $format_str, %rdi\n";
                        }
                        
                        assembly << "    xor %rax, %rax\n";
                        assembly << "    call printf\n";
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
        auto it = variables.find(node.name);
        if (it != variables.end()) {
            assembly << "    mov -" << it->second << "(%rbp), %rax\n";
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
        
        // Step 1: Lexical analysis
        orion::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        // Step 2: Parsing
        orion::SimpleOrionParser parser(tokens);
        auto ast = parser.parse();
        
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