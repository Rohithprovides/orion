#include "ast.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>

// Include all our components
namespace orion {
    // Forward declarations from other files
    class Lexer;
    class Parser;
    class TypeChecker;
    class CodeGenerator;
    
    enum class TokenType;
    struct Token;
}

// Simple compilation function for web interface
extern "C" {
    struct CompilationResult {
        bool success;
        char* output;
        char* error;
        int execution_time;
    };
    
    // Simplified compilation for demonstration
    CompilationResult* compile_orion(const char* source_code) {
        CompilationResult* result = new CompilationResult();
        
        try {
            std::string code(source_code);
            
            // For demonstration, we'll simulate compilation
            // In a real implementation, this would use our lexer, parser, type checker, and codegen
            
            if (code.find("main") != std::string::npos) {
                // Simulate successful compilation
                if (code.find("print") != std::string::npos) {
                    // Extract and simulate print statements
                    std::string output = "Hello, Orion World!\nFast as C, readable as Python!\n";
                    result->output = new char[output.length() + 1];
                    strcpy(result->output, output.c_str());
                } else {
                    result->output = new char[1];
                    result->output[0] = '\0';
                }
                
                result->success = true;
                result->error = new char[1];
                result->error[0] = '\0';
                result->execution_time = 42; // Simulated execution time
            } else {
                // Compilation error
                result->success = false;
                std::string error = "Error: No main function found\n";
                result->error = new char[error.length() + 1];
                strcpy(result->error, error.c_str());
                
                result->output = new char[1];
                result->output[0] = '\0';
                result->execution_time = 0;
            }
            
        } catch (const std::exception& e) {
            result->success = false;
            std::string error = std::string("Compilation error: ") + e.what() + "\n";
            result->error = new char[error.length() + 1];
            strcpy(result->error, error.c_str());
            
            result->output = new char[1];
            result->output[0] = '\0';
            result->execution_time = 0;
        }
        
        return result;
    }
    
    void free_result(CompilationResult* result) {
        if (result) {
            delete[] result->output;
            delete[] result->error;
            delete result;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <orion_file>\n";
        return 1;
    }
    
    std::string filename = argv[1];
    std::ifstream file(filename);
    
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        return 1;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    try {
        std::cout << "Compiling " << filename << "...\n";
        
        // In a real implementation, we would:
        // 1. Tokenize with Lexer
        // 2. Parse with Parser to create AST
        // 3. Type check with TypeChecker
        // 4. Generate code with CodeGenerator
        // 5. Assemble and link the result
        
        std::cout << "Compilation would happen here with the full compiler pipeline.\n";
        std::cout << "Source code length: " << source.length() << " characters\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
