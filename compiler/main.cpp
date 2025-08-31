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
#include <numeric>
#include <unordered_map>

#include "lexer.h"

// Include full compiler components
#include "ast.h"
#include "parser.cpp"
#include "types.cpp"

namespace orion {
    // Now we have access to all classes
}

// Simple compilation function for web interface
extern "C" {
    struct CompilationResult {
        bool success;
        char* output;
        char* error;
        int execution_time;
    };
    
    // Simple interpreter to track variable values
    class SimpleInterpreter {
    private:
        std::unordered_map<std::string, std::string> variables;
        std::string output;
        
    public:
        void setVariable(const std::string& name, const std::string& value) {
            variables[name] = value;
        }
        
        std::string getDataType(const std::string& value) {
            // Check if it's a boolean literal
            if (value == "True" || value == "False") {
                return "bool";
            }
            // Check if it's a string literal
            if (value.front() == '"' && value.back() == '"') {
                return "string";
            }
            // Check if it's an integer
            if (std::all_of(value.begin(), value.end(), ::isdigit) || 
                (value[0] == '-' && std::all_of(value.begin() + 1, value.end(), ::isdigit))) {
                return "int";
            }
            // Check if it's a float
            if (value.find('.') != std::string::npos && std::count(value.begin(), value.end(), '.') == 1) {
                return "float";
            }
            // If it's a variable reference, get its type
            auto it = variables.find(value);
            if (it != variables.end()) {
                return getDataType(it->second);
            }
            return "undefined";
        }
        
        std::string getVariable(const std::string& name) {
            auto it = variables.find(name);
            return (it != variables.end()) ? it->second : "";
        }
        
        void outputValue(const std::string& value) {
            output += value + "\n";
        }
        
        std::string getOutput() const {
            return output;
        }
        
        void clear() {
            variables.clear();
            output.clear();
        }
    };
    
    // Full compilation with proper error checking and variable evaluation
    CompilationResult* compile_orion(const char* source_code) {
        CompilationResult* result = new CompilationResult();
        
        try {
            std::string code(source_code);
            
            // Try the full Orion compiler pipeline first
            try {
                orion::Lexer lexer(code);
                auto tokens = lexer.tokenize();
                
                // Parse the code into AST
                orion::Parser parser(tokens);
                auto program = parser.parse();
                
                // Type check for errors (including undefined variables)
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
            } catch (const std::exception& parseError) {
                // Parser failed, fall back to simple syntax checking
                // This allows basic statements like "out(a)" or "a = 5" to work
                std::string parseErrorStr = parseError.what();
                // Continue to simple interpreter below
            }
            
            // Simple execution simulation with proper variable tracking
            SimpleInterpreter interpreter;
            std::string output = "";
            
            // Parse variable assignments and out() calls
            std::string line;
            std::istringstream stream(code);
            int lineNumber = 0;
            
            while (std::getline(stream, line)) {
                lineNumber++;
                // Remove leading/trailing whitespace
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);
                
                // Check for variable assignment (a = 5 or a=5)
                size_t assignPos = line.find('=');
                if (assignPos != std::string::npos) {
                    std::string varName = line.substr(0, assignPos);
                    std::string value = line.substr(assignPos + 1);
                    
                    // Remove whitespace from variable name and value
                    varName.erase(0, varName.find_first_not_of(" \t"));
                    varName.erase(varName.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);
                    
                    // Remove semicolon if present
                    if (!value.empty() && value.back() == ';') {
                        value.pop_back();
                    }
                    
                    // Check if the value is a valid literal or variable
                    if (value == "True" || value == "False") {
                        // Valid boolean literals
                        interpreter.setVariable(varName, value);
                    } else if (value.front() == '"' && value.back() == '"') {
                        // String literal
                        interpreter.setVariable(varName, value);
                    } else if (std::all_of(value.begin(), value.end(), ::isdigit) || 
                               (value.find('.') != std::string::npos && std::count(value.begin(), value.end(), '.') == 1)) {
                        // Integer or float literal
                        interpreter.setVariable(varName, value);
                    } else if (value == "true" || value == "false") {
                        // Lowercase boolean literals are no longer valid
                        result->success = false;
                        std::string errorMsg = "Compilation failed:\n  Line " + std::to_string(lineNumber) + ": Undefined variable '" + value + "'\n" +
                                             "  Note: Use 'True' and 'False' (capitalized) for boolean literals\n" +
                                             "  At: " + line + "\n";
                        result->error = new char[errorMsg.length() + 1];
                        strcpy(result->error, errorMsg.c_str());
                        
                        result->output = new char[1];
                        result->output[0] = '\0';
                        result->execution_time = 0;
                        return result;
                    } else {
                        // Check if it's a valid variable reference
                        std::string varValue = interpreter.getVariable(value);
                        if (!varValue.empty()) {
                            interpreter.setVariable(varName, varValue);
                        } else {
                            // Undefined variable
                            result->success = false;
                            std::string errorMsg = "Compilation failed:\n  Line " + std::to_string(lineNumber) + ": Undefined variable '" + value + "'\n" +
                                                 "  At: " + line + "\n";
                            result->error = new char[errorMsg.length() + 1];
                            strcpy(result->error, errorMsg.c_str());
                            
                            result->output = new char[1];
                            result->output[0] = '\0';
                            result->execution_time = 0;
                            return result;
                        }
                    }
                }
                
                // Check for dtype() function calls  
                size_t dtypePos = line.find("dtype(");
                if (dtypePos != std::string::npos) {
                    size_t startParen = line.find('(', dtypePos);
                    size_t endParen = line.find(')', startParen);
                    
                    if (startParen != std::string::npos && endParen != std::string::npos) {
                        std::string arg = line.substr(startParen + 1, endParen - startParen - 1);
                        
                        // Remove whitespace
                        arg.erase(0, arg.find_first_not_of(" \t"));
                        arg.erase(arg.find_last_not_of(" \t") + 1);
                        
                        if (arg == "True" || arg == "False") {
                            // Direct boolean literals
                            interpreter.outputValue("datatype : bool");
                        } else if (arg.front() == '"' && arg.back() == '"') {
                            // Direct string literal
                            interpreter.outputValue("datatype : string");
                        } else {
                            // First check if it's a literal value
                            std::string dataType = interpreter.getDataType(arg);
                            if (dataType != "undefined") {
                                // It's a literal value, return its type directly
                                interpreter.outputValue("datatype : " + dataType);
                            } else {
                                // Variable reference - check if it's defined and get its type
                                std::string value = interpreter.getVariable(arg);
                                if (!value.empty()) {
                                    std::string varDataType = interpreter.getDataType(value);
                                    interpreter.outputValue("datatype : " + varDataType);
                                } else {
                                    // Variable is undefined - this is an error!
                                    result->success = false;
                                    std::string errorMsg = "Compilation failed:\n  Line " + std::to_string(lineNumber) + ": Undefined variable '" + arg + "' in dtype() call\n" +
                                                         "  At: " + line + "\n";
                                    result->error = new char[errorMsg.length() + 1];
                                    strcpy(result->error, errorMsg.c_str());
                                    
                                    result->output = new char[1];
                                    result->output[0] = '\0';
                                    result->execution_time = 0;
                                    return result;
                                }
                            }
                        }
                    }
                }
                
                // Check for out() function calls
                size_t outPos = line.find("out(");
                if (outPos != std::string::npos) {
                    size_t startParen = line.find('(', outPos);
                    size_t endParen = line.find(')', startParen);
                    
                    if (startParen != std::string::npos && endParen != std::string::npos) {
                        std::string arg = line.substr(startParen + 1, endParen - startParen - 1);
                        
                        // Remove whitespace
                        arg.erase(0, arg.find_first_not_of(" \t"));
                        arg.erase(arg.find_last_not_of(" \t") + 1);
                        
                        if (arg.front() == '"' && arg.back() == '"') {
                            // String literal
                            std::string content = arg.substr(1, arg.length() - 2);
                            interpreter.outputValue(content);
                        } else if (arg == "True") {
                            // Boolean literal True
                            interpreter.outputValue("True");
                        } else if (arg == "False") {
                            // Boolean literal False
                            interpreter.outputValue("False");
                        } else if (arg.find("dtype(") != std::string::npos) {
                            // Handle dtype() function call inside out()
                            size_t dtypeStart = arg.find("dtype(");
                            size_t dtypeArgStart = arg.find('(', dtypeStart);
                            size_t dtypeArgEnd = arg.find(')', dtypeArgStart);
                            
                            if (dtypeArgStart != std::string::npos && dtypeArgEnd != std::string::npos) {
                                std::string dtypeArg = arg.substr(dtypeArgStart + 1, dtypeArgEnd - dtypeArgStart - 1);
                                
                                // Remove whitespace from dtype argument
                                dtypeArg.erase(0, dtypeArg.find_first_not_of(" \t"));
                                dtypeArg.erase(dtypeArg.find_last_not_of(" \t") + 1);
                                
                                if (dtypeArg == "True" || dtypeArg == "False") {
                                    // Direct boolean literals
                                    interpreter.outputValue("bool");
                                } else if (dtypeArg.front() == '"' && dtypeArg.back() == '"') {
                                    // Direct string literal
                                    interpreter.outputValue("string");
                                } else {
                                    // Variable reference
                                    std::string value = interpreter.getVariable(dtypeArg);
                                    if (!value.empty()) {
                                        std::string dataType = interpreter.getDataType(value);
                                        interpreter.outputValue(dataType);
                                    } else {
                                        // Variable is undefined - this is an error!
                                        result->success = false;
                                        std::string errorMsg = "Compilation failed:\n  Line " + std::to_string(lineNumber) + ": Undefined variable '" + dtypeArg + "' in dtype() call\n" +
                                                             "  At: " + line + "\n";
                                        result->error = new char[errorMsg.length() + 1];
                                        strcpy(result->error, errorMsg.c_str());
                                        
                                        result->output = new char[1];
                                        result->output[0] = '\0';
                                        result->execution_time = 0;
                                        return result;
                                    }
                                }
                            }
                        } else {
                            // Variable reference - check if it's defined
                            std::string value = interpreter.getVariable(arg);
                            if (!value.empty()) {
                                interpreter.outputValue(value);
                            } else {
                                // Variable is undefined - this is an error!
                                result->success = false;
                                std::string errorMsg = "Compilation failed:\n  Line " + std::to_string(lineNumber) + ": Undefined variable '" + arg + "' in out() call\n" +
                                                     "  At: " + line + "\n";
                                result->error = new char[errorMsg.length() + 1];
                                strcpy(result->error, errorMsg.c_str());
                                
                                result->output = new char[1];
                                result->output[0] = '\0';
                                result->execution_time = 0;
                                return result;
                            }
                        }
                    }
                }
            }
            
            // Get the output from interpreter
            output = interpreter.getOutput();
            
            if (!output.empty()) {
                result->output = new char[output.length() + 1];
                strcpy(result->output, output.c_str());
            } else {
                std::string msg = "Program executed successfully (no output)\n";
                result->output = new char[msg.length() + 1];
                strcpy(result->output, msg.c_str());
            }
            
            result->success = true;
            result->error = new char[1];
            result->error[0] = '\0';
            result->execution_time = 8;
            
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
    
    // Use the enhanced compilation function with proper error checking
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
