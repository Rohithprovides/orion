#include "ast.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <cstdio>

#include "lexer.h"

// Include full compiler components
#include "ast.h"
#include "parser.cpp"
#include "types.cpp"
#include "codegen.cpp"

namespace orion {
    // Now we have access to all classes
}

// Compilation function for web interface
extern "C" {
    struct CompilationResult {
        bool success;
        char* output;
        char* error;
        int execution_time;
    };
    
    // Pure compilation function - no interpretation
    CompilationResult* compile_orion(const char* source_code) {
        CompilationResult* result = new CompilationResult();
        
        try {
            std::string code(source_code);
            
            // Full Orion compiler pipeline
            orion::Lexer lexer(code);
            auto tokens = lexer.tokenize();
            
            // Parse the code into AST
            orion::Parser parser(tokens);
            auto program = parser.parse();
            
            // Type check for errors
            orion::TypeChecker typeChecker;
            bool typeCheckPassed = typeChecker.check(*program);
            
            if (!typeCheckPassed) {
                // Compilation failed due to type errors
                result->success = false;
                std::string errorMsg = "Compilation failed:\n";
                for (const auto& error : typeChecker.getErrors()) {
                    errorMsg += "  " + error + "\n";
                }
                result->error = new char[errorMsg.length() + 1];
                strcpy(result->error, errorMsg.c_str());
                
                result->output = new char[1];
                result->output[0] = '\0';
                result->execution_time = 0;
                return result;
            }
            
            // Generate assembly code
            orion::CodeGenerator codeGen;
            std::string assembly = codeGen.generate(*program);
            
            // For now, create a temporary assembly file and compile it
            std::string tempAsmFile = "/tmp/orion_temp.s";
            std::string tempExeFile = "/tmp/orion_temp";
            
            // Write assembly to file
            std::ofstream asmFile(tempAsmFile);
            if (!asmFile) {
                result->success = false;
                std::string errorMsg = "Error: Could not create temporary assembly file\n";
                result->error = new char[errorMsg.length() + 1];
                strcpy(result->error, errorMsg.c_str());
                
                result->output = new char[1];
                result->output[0] = '\0';
                result->execution_time = 0;
                return result;
            }
            
            asmFile << assembly;
            asmFile.close();
            
            // Compile assembly to executable
            std::string gccCommand = "gcc -o " + tempExeFile + " " + tempAsmFile + " 2>&1";
            FILE* pipe = popen(gccCommand.c_str(), "r");
            if (!pipe) {
                result->success = false;
                std::string errorMsg = "Error: Could not execute assembler\n";
                result->error = new char[errorMsg.length() + 1];
                strcpy(result->error, errorMsg.c_str());
                
                result->output = new char[1];
                result->output[0] = '\0';
                result->execution_time = 0;
                return result;
            }
            
            std::string gccOutput;
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                gccOutput += buffer;
            }
            int gccResult = pclose(pipe);
            
            if (gccResult != 0) {
                // Assembly compilation failed
                result->success = false;
                std::string errorMsg = "Assembly compilation failed:\n" + gccOutput;
                result->error = new char[errorMsg.length() + 1];
                strcpy(result->error, errorMsg.c_str());
                
                result->output = new char[1];
                result->output[0] = '\0';
                result->execution_time = 0;
                
                // Clean up temporary files
                remove(tempAsmFile.c_str());
                return result;
            }
            
            // Execute the compiled program
            FILE* execPipe = popen(tempExeFile.c_str(), "r");
            if (!execPipe) {
                result->success = false;
                std::string errorMsg = "Error: Could not execute compiled program\n";
                result->error = new char[errorMsg.length() + 1];
                strcpy(result->error, errorMsg.c_str());
                
                result->output = new char[1];
                result->output[0] = '\0';
                result->execution_time = 0;
                
                // Clean up temporary files
                remove(tempAsmFile.c_str());
                remove(tempExeFile.c_str());
                return result;
            }
            
            std::string programOutput;
            while (fgets(buffer, sizeof(buffer), execPipe) != NULL) {
                programOutput += buffer;
            }
            pclose(execPipe);
            
            // Clean up temporary files
            remove(tempAsmFile.c_str());
            remove(tempExeFile.c_str());
            
            // Success
            result->success = true;
            result->error = new char[1];
            result->error[0] = '\0';
            
            if (programOutput.empty()) {
                programOutput = "Program compiled and executed successfully";
            }
            
            result->output = new char[programOutput.length() + 1];
            strcpy(result->output, programOutput.c_str());
            result->execution_time = 50; // Placeholder execution time
            
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
    
    // Use the pure compilation function
    CompilationResult* result = compile_orion(source.c_str());
    
    if (result->success) {
        std::cout << result->output;
        free_result(result);
        return 0;
    } else {
        std::cerr << result->error;
        free_result(result);
        return 1;
    }
}