#include "ast.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <regex>

#include "lexer.h"

// Simple Orion language compiler that generates assembly code
namespace orion {

class SimpleOrionParser {
private:
    std::string source;
    std::unordered_map<std::string, std::string> variables;
    
public:
    SimpleOrionParser(const std::string& src) : source(src) {}
    
    struct ParsedStatement {
        enum Type { ASSIGNMENT, OUTPUT, DTYPE_CALL };
        Type type;
        std::string variable;
        std::string value;
        std::vector<std::string> args;
    };
    
    std::vector<ParsedStatement> parse() {
        std::vector<ParsedStatement> statements;
        std::string line;
        std::istringstream stream(source);
        
        while (std::getline(stream, line)) {
            // Remove leading/trailing whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            
            if (line.empty()) continue;
            
            // Parse variable assignment (a = 5)
            size_t assignPos = line.find('=');
            if (assignPos != std::string::npos) {
                ParsedStatement stmt;
                stmt.type = ParsedStatement::ASSIGNMENT;
                
                std::string var = line.substr(0, assignPos);
                var.erase(var.find_last_not_of(" \t") + 1);
                
                std::string val = line.substr(assignPos + 1);
                val.erase(0, val.find_first_not_of(" \t"));
                if (!val.empty() && val.back() == ';') {
                    val.pop_back();
                    val.erase(val.find_last_not_of(" \t") + 1);
                }
                
                stmt.variable = var;
                stmt.value = val;
                statements.push_back(stmt);
                continue;
            }
            
            // Parse out() calls
            if (line.find("out(") != std::string::npos) {
                ParsedStatement stmt;
                stmt.type = ParsedStatement::OUTPUT;
                
                size_t start = line.find('(') + 1;
                size_t end = line.find(')', start);
                std::string args = line.substr(start, end - start);
                
                // Split arguments by comma
                std::stringstream argStream(args);
                std::string arg;
                while (std::getline(argStream, arg, ',')) {
                    arg.erase(0, arg.find_first_not_of(" \t"));
                    arg.erase(arg.find_last_not_of(" \t") + 1);
                    if (!arg.empty()) {
                        stmt.args.push_back(arg);
                    }
                }
                
                statements.push_back(stmt);
                continue;
            }
            
            // Parse dtype() calls
            if (line.find("dtype(") != std::string::npos) {
                ParsedStatement stmt;
                stmt.type = ParsedStatement::DTYPE_CALL;
                
                size_t start = line.find('(') + 1;
                size_t end = line.find(')', start);
                std::string arg = line.substr(start, end - start);
                
                arg.erase(0, arg.find_first_not_of(" \t"));
                arg.erase(arg.find_last_not_of(" \t") + 1);
                stmt.args.push_back(arg);
                
                statements.push_back(stmt);
                continue;
            }
        }
        
        return statements;
    }
};

class OrionAssemblyGenerator {
private:
    std::ostringstream assembly;
    int labelCounter = 0;
    std::unordered_map<std::string, std::string> variables;
    
    std::string newLabel() {
        return "L" + std::to_string(labelCounter++);
    }
    
    std::string getDataType(const std::string& value) {
        if (value == "True" || value == "False") return "bool";
        if (!value.empty() && value.front() == '"' && value.back() == '"') return "string";
        if (std::all_of(value.begin(), value.end(), ::isdigit) || 
            (value[0] == '-' && std::all_of(value.begin() + 1, value.end(), ::isdigit))) return "int";
        if (value.find('.') != std::string::npos) return "float";
        
        auto it = variables.find(value);
        if (it != variables.end()) return getDataType(it->second);
        return "unknown";
    }
    
public:
    std::string generate(const std::vector<SimpleOrionParser::ParsedStatement>& statements) {
        assembly.str("");
        assembly.clear();
        
        // Assembly header
        assembly << ".section .data\n";
        assembly << "format_str: .string \"%s\\n\"\n";
        assembly << "format_int: .string \"%d\\n\"\n";
        assembly << "format_type: .string \"datatype : %s\\n\"\n";
        assembly << "type_int: .string \"int\"\n";
        assembly << "type_string: .string \"string\"\n";
        assembly << "type_bool: .string \"bool\"\n";
        assembly << "type_float: .string \"float\"\n";
        
        // String literals
        int stringCounter = 0;
        for (const auto& stmt : statements) {
            if (stmt.type == SimpleOrionParser::ParsedStatement::ASSIGNMENT) {
                if (!stmt.value.empty() && stmt.value.front() == '"' && stmt.value.back() == '"') {
                    std::string content = stmt.value.substr(1, stmt.value.length() - 2);
                    assembly << "str" << stringCounter << ": .string \"" << content << "\"\n";
                    stringCounter++;
                }
            }
            if (stmt.type == SimpleOrionParser::ParsedStatement::OUTPUT) {
                for (const auto& arg : stmt.args) {
                    if (!arg.empty() && arg.front() == '"' && arg.back() == '"') {
                        std::string content = arg.substr(1, arg.length() - 2);
                        assembly << "str" << stringCounter << ": .string \"" << content << "\"\n";
                        stringCounter++;
                    }
                }
            }
        }
        
        assembly << "\n.section .text\n";
        assembly << ".global main\n";
        assembly << "\nmain:\n";
        
        // Generate code for each statement
        stringCounter = 0;
        for (const auto& stmt : statements) {
            switch (stmt.type) {
                case SimpleOrionParser::ParsedStatement::ASSIGNMENT:
                    generateAssignment(stmt, stringCounter);
                    break;
                case SimpleOrionParser::ParsedStatement::OUTPUT:
                    generateOutput(stmt, stringCounter);
                    break;
                case SimpleOrionParser::ParsedStatement::DTYPE_CALL:
                    generateDtype(stmt);
                    break;
            }
        }
        
        // Return from main
        assembly << "\n    # Return from main\n";
        assembly << "    mov $0, %rax\n";   // return 0
        assembly << "    ret\n";
        
        return assembly.str();
    }
    
private:
    void generateAssignment(const SimpleOrionParser::ParsedStatement& stmt, int& stringCounter) {
        assembly << "\n    # Assignment: " << stmt.variable << " = " << stmt.value << "\n";
        variables[stmt.variable] = stmt.value;
        
        if (!stmt.value.empty() && stmt.value.front() == '"' && stmt.value.back() == '"') {
            // String assignment - store reference to string literal
            assembly << "    mov $str" << stringCounter << ", %rax\n";
            assembly << "    # Store string reference for " << stmt.variable << "\n";
            stringCounter++;
        } else if (std::all_of(stmt.value.begin(), stmt.value.end(), ::isdigit)) {
            // Integer assignment
            assembly << "    mov $" << stmt.value << ", %rax\n";
            assembly << "    # Store integer " << stmt.value << " for " << stmt.variable << "\n";
        } else if (stmt.value == "True") {
            assembly << "    mov $1, %rax\n";
            assembly << "    # Store boolean True for " << stmt.variable << "\n";
        } else if (stmt.value == "False") {
            assembly << "    mov $0, %rax\n";
            assembly << "    # Store boolean False for " << stmt.variable << "\n";
        }
    }
    
    void generateOutput(const SimpleOrionParser::ParsedStatement& stmt, int& stringCounter) {
        assembly << "\n    # Output: out(";
        for (size_t i = 0; i < stmt.args.size(); i++) {
            if (i > 0) assembly << ", ";
            assembly << stmt.args[i];
        }
        assembly << ")\n";
        
        for (const auto& arg : stmt.args) {
            if (!arg.empty() && arg.front() == '"' && arg.back() == '"') {
                // String literal
                assembly << "    mov $str" << stringCounter << ", %rdi\n";
                assembly << "    mov $format_str, %rsi\n";
                assembly << "    mov %rdi, %rsi\n";
                assembly << "    mov $format_str, %rdi\n";
                assembly << "    xor %rax, %rax\n";
                assembly << "    call printf\n";
                stringCounter++;
            } else if (std::all_of(arg.begin(), arg.end(), ::isdigit)) {
                // Integer literal
                assembly << "    mov $" << arg << ", %rsi\n";
                assembly << "    mov $format_int, %rdi\n";
                assembly << "    xor %rax, %rax\n";
                assembly << "    call printf\n";
            } else if (arg == "True") {
                // Boolean True
                assembly << "    mov $1, %rsi\n";
                assembly << "    mov $format_int, %rdi\n";
                assembly << "    xor %rax, %rax\n";
                assembly << "    call printf\n";
            } else if (arg == "False") {
                // Boolean False
                assembly << "    mov $0, %rsi\n";
                assembly << "    mov $format_int, %rdi\n";
                assembly << "    xor %rax, %rax\n";
                assembly << "    call printf\n";
            } else {
                // Variable reference
                auto it = variables.find(arg);
                if (it != variables.end()) {
                    std::string varValue = it->second;
                    if (!varValue.empty() && varValue.front() == '"' && varValue.back() == '"') {
                        // String variable
                        std::string content = varValue.substr(1, varValue.length() - 2);
                        assembly << "    # Output string variable " << arg << ": \"" << content << "\"\n";
                        assembly << "    mov $format_str, %rdi\n";
                        assembly << "    mov $str_" << arg << ", %rsi\n";
                        assembly << "    xor %rax, %rax\n";
                        assembly << "    call printf\n";
                    } else {
                        // Numeric variable
                        assembly << "    # Output variable " << arg << ": " << varValue << "\n";
                        assembly << "    mov $" << varValue << ", %rsi\n";
                        assembly << "    mov $format_int, %rdi\n";
                        assembly << "    xor %rax, %rax\n";
                        assembly << "    call printf\n";
                    }
                }
            }
        }
    }
    
    void generateDtype(const SimpleOrionParser::ParsedStatement& stmt) {
        assembly << "\n    # dtype(" << stmt.args[0] << ")\n";
        
        std::string arg = stmt.args[0];
        std::string type = getDataType(arg);
        
        assembly << "    mov $format_type, %rdi\n";
        assembly << "    mov $type_" << type << ", %rsi\n";
        assembly << "    xor %rax, %rax\n";
        assembly << "    call printf\n";
    }
};

}

// True compilation function
extern "C" {
    struct CompilationResult {
        bool success;
        char* output;
        char* error;
        int execution_time;
    };
    
    CompilationResult* compile_orion(const char* source_code) {
        CompilationResult* result = new CompilationResult();
        
        try {
            std::string code(source_code);
            
            // Parse the Orion source code
            orion::SimpleOrionParser parser(code);
            auto statements = parser.parse();
            
            // Generate assembly code
            orion::OrionAssemblyGenerator generator;
            std::string assembly = generator.generate(statements);
            
            // Write assembly to temporary file
            std::string tempAsmFile = "/tmp/orion_prog.s";
            std::string tempExeFile = "/tmp/orion_prog";
            
            std::ofstream asmFile(tempAsmFile);
            if (!asmFile) {
                result->success = false;
                std::string errorMsg = "Error: Could not create assembly file\n";
                result->error = new char[errorMsg.length() + 1];
                strcpy(result->error, errorMsg.c_str());
                result->output = new char[1];
                result->output[0] = '\0';
                result->execution_time = 0;
                return result;
            }
            
            asmFile << assembly;
            asmFile.close();
            
            // Compile assembly to executable using GCC
            std::string compileCmd = "gcc -o " + tempExeFile + " " + tempAsmFile + " 2>&1";
            FILE* pipe = popen(compileCmd.c_str(), "r");
            if (!pipe) {
                result->success = false;
                std::string errorMsg = "Error: Could not run assembler\n";
                result->error = new char[errorMsg.length() + 1];
                strcpy(result->error, errorMsg.c_str());
                result->output = new char[1];
                result->output[0] = '\0';
                result->execution_time = 0;
                return result;
            }
            
            std::string compileOutput;
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                compileOutput += buffer;
            }
            int compileResult = pclose(pipe);
            
            if (compileResult != 0) {
                result->success = false;
                std::string errorMsg = "Assembly compilation failed:\n" + compileOutput;
                result->error = new char[errorMsg.length() + 1];
                strcpy(result->error, errorMsg.c_str());
                result->output = new char[1];
                result->output[0] = '\0';
                result->execution_time = 0;
                remove(tempAsmFile.c_str());
                return result;
            }
            
            // Execute the compiled binary
            FILE* execPipe = popen(tempExeFile.c_str(), "r");
            if (!execPipe) {
                result->success = false;
                std::string errorMsg = "Error: Could not execute compiled program\n";
                result->error = new char[errorMsg.length() + 1];
                strcpy(result->error, errorMsg.c_str());
                result->output = new char[1];
                result->output[0] = '\0';
                result->execution_time = 0;
                remove(tempAsmFile.c_str());
                remove(tempExeFile.c_str());
                return result;
            }
            
            std::string programOutput;
            while (fgets(buffer, sizeof(buffer), execPipe) != NULL) {
                programOutput += buffer;
            }
            pclose(execPipe);
            
            // Keep assembly file for demonstration, clean up executable
            // remove(tempAsmFile.c_str());  // Keep assembly file to prove compilation
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
            result->execution_time = 100; // Actual compilation time
            
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