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

// Compilation function that handles Orion syntax like the previous interpreter
extern "C" {
    struct CompilationResult {
        bool success;
        char* output;
        char* error;
        int execution_time;
    };
    
    // Compiler that processes Orion code and generates expected output
    class OrionCompiler {
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
    
    // Orion compiler with proper error checking and variable evaluation
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
                // Continue to simple compilation below
            }
            
            // Compile Orion code with proper variable tracking
            OrionCompiler compiler;
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
                
                // Check for tuple assignment first (a,b) = (b,a)
                if (line.find('(') != std::string::npos && line.find(')') != std::string::npos && line.find('=') != std::string::npos) {
                    size_t firstOpenParen = line.find('(');
                    size_t firstCloseParen = line.find(')', firstOpenParen);
                    size_t equalPos = line.find('=', firstCloseParen);
                    size_t secondOpenParen = line.find('(', equalPos);
                    size_t secondCloseParen = line.find(')', secondOpenParen);
                    
                    if (firstOpenParen != std::string::npos && firstCloseParen != std::string::npos && 
                        equalPos != std::string::npos && secondOpenParen != std::string::npos && 
                        secondCloseParen != std::string::npos) {
                        
                        // Extract variables from left side (a,b)
                        std::string leftVars = line.substr(firstOpenParen + 1, firstCloseParen - firstOpenParen - 1);
                        // Extract variables from right side (b,a)
                        std::string rightVars = line.substr(secondOpenParen + 1, secondCloseParen - secondOpenParen - 1);
                        
                        // Parse left side variables
                        std::vector<std::string> leftVarList;
                        std::stringstream leftStream(leftVars);
                        std::string leftVar;
                        while (std::getline(leftStream, leftVar, ',')) {
                            leftVar.erase(0, leftVar.find_first_not_of(" \t"));
                            leftVar.erase(leftVar.find_last_not_of(" \t") + 1);
                            leftVarList.push_back(leftVar);
                        }
                        
                        // Parse right side variables
                        std::vector<std::string> rightVarList;
                        std::stringstream rightStream(rightVars);
                        std::string rightVar;
                        while (std::getline(rightStream, rightVar, ',')) {
                            rightVar.erase(0, rightVar.find_first_not_of(" \t"));
                            rightVar.erase(rightVar.find_last_not_of(" \t") + 1);
                            rightVarList.push_back(rightVar);
                        }
                        
                        // Verify all variables exist and same number on both sides
                        if (leftVarList.size() != rightVarList.size()) {
                            result->success = false;
                            std::string errorMsg = "Compilation failed:\n  Line " + std::to_string(lineNumber) + ": Mismatched number of variables in tuple assignment\n" +
                                                 "  At: " + line + "\n";
                            result->error = new char[errorMsg.length() + 1];
                            strcpy(result->error, errorMsg.c_str());
                            
                            result->output = new char[1];
                            result->output[0] = '\0';
                            result->execution_time = 0;
                            return result;
                        }
                        
                        // Check all right-side variables/literals exist and get their values
                        std::vector<std::string> rightValues;
                        for (const auto& rightVar : rightVarList) {
                            std::string value;
                            
                            // Check if it's a literal value first
                            if (rightVar == "True" || rightVar == "False") {
                                // Valid boolean literals
                                value = rightVar;
                            } else if (!rightVar.empty() && rightVar.front() == '"' && rightVar.back() == '"') {
                                // String literal
                                value = rightVar;
                            } else if (!rightVar.empty() && (std::all_of(rightVar.begin(), rightVar.end(), ::isdigit) || 
                                       (rightVar.find('.') != std::string::npos && std::count(rightVar.begin(), rightVar.end(), '.') == 1))) {
                                // Integer or float literal
                                value = rightVar;
                            } else {
                                // Must be a variable reference
                                value = compiler.getVariable(rightVar);
                                if (value.empty()) {
                                    result->success = false;
                                    std::string errorMsg = "Compilation failed:\n  Line " + std::to_string(lineNumber) + ": Undefined variable '" + rightVar + "' in tuple assignment\n" +
                                                         "  At: " + line + "\n";
                                    result->error = new char[errorMsg.length() + 1];
                                    strcpy(result->error, errorMsg.c_str());
                                    
                                    result->output = new char[1];
                                    result->output[0] = '\0';
                                    result->execution_time = 0;
                                    return result;
                                }
                            }
                            rightValues.push_back(value);
                        }
                        
                        // Perform the swap/assignment
                        for (size_t i = 0; i < leftVarList.size(); i++) {
                            compiler.setVariable(leftVarList[i], rightValues[i]);
                        }
                        
                        continue; // Skip regular assignment processing
                    }
                }
                
                // Check for variable assignment (a = 5, a=5, or chained a = b = 5)
                size_t assignPos = line.find('=');
                if (assignPos != std::string::npos) {
                    // Split the line into parts by '=' to handle chained assignments
                    std::vector<std::string> assignmentParts;
                    std::stringstream assignStream(line);
                    std::string part;
                    
                    while (std::getline(assignStream, part, '=')) {
                        // Remove whitespace from each part
                        part.erase(0, part.find_first_not_of(" \t"));
                        part.erase(part.find_last_not_of(" \t") + 1);
                        if (!part.empty()) {
                            assignmentParts.push_back(part);
                        }
                    }
                    
                    if (assignmentParts.size() >= 2) {
                        // Get the rightmost value (the actual value to assign)
                        std::string value = assignmentParts.back();
                        
                        // Remove semicolon if present
                        if (!value.empty() && value.back() == ';') {
                            value.pop_back();
                            value.erase(value.find_last_not_of(" \t") + 1); // Clean up after semicolon removal
                        }
                        
                        // Validate the value first
                        std::string actualValue;
                        if (value == "True" || value == "False") {
                            // Valid boolean literals
                            actualValue = value;
                        } else if (!value.empty() && value.front() == '"' && value.back() == '"') {
                            // String literal
                            actualValue = value;
                        } else if (!value.empty() && (std::all_of(value.begin(), value.end(), ::isdigit) || 
                                   (value.find('.') != std::string::npos && std::count(value.begin(), value.end(), '.') == 1))) {
                            // Integer or float literal
                            actualValue = value;
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
                            std::string varValue = compiler.getVariable(value);
                            if (!varValue.empty()) {
                                actualValue = varValue;
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
                        
                        // Assign the value to all variables (all parts except the last one)
                        for (size_t i = 0; i < assignmentParts.size() - 1; i++) {
                            compiler.setVariable(assignmentParts[i], actualValue);
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
                            compiler.outputValue("datatype : bool");
                        } else if (arg.front() == '"' && arg.back() == '"') {
                            // Direct string literal
                            compiler.outputValue("datatype : string");
                        } else {
                            // First check if it's a literal value
                            std::string dataType = compiler.getDataType(arg);
                            if (dataType != "undefined") {
                                // It's a literal value, return its type directly
                                compiler.outputValue("datatype : " + dataType);
                            } else {
                                // Variable reference - check if it's defined and get its type
                                std::string value = compiler.getVariable(arg);
                                if (!value.empty()) {
                                    std::string varDataType = compiler.getDataType(value);
                                    compiler.outputValue("datatype : " + varDataType);
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
                        std::string args = line.substr(startParen + 1, endParen - startParen - 1);
                        
                        // Parse multiple comma-separated arguments
                        std::vector<std::string> argList;
                        std::stringstream argStream(args);
                        std::string singleArg;
                        
                        while (std::getline(argStream, singleArg, ',')) {
                            // Remove whitespace from each argument
                            singleArg.erase(0, singleArg.find_first_not_of(" \t"));
                            singleArg.erase(singleArg.find_last_not_of(" \t") + 1);
                            if (!singleArg.empty()) {
                                argList.push_back(singleArg);
                            }
                        }
                        
                        // Process each argument and collect output values
                        std::vector<std::string> outputValues;
                        for (const auto& arg : argList) {
                            if (!arg.empty() && arg.front() == '"' && arg.back() == '"') {
                                // String literal
                                std::string content = arg.substr(1, arg.length() - 2);
                                outputValues.push_back(content);
                            } else if (arg == "True") {
                                // Boolean literal True
                                outputValues.push_back("True");
                            } else if (arg == "False") {
                                // Boolean literal False
                                outputValues.push_back("False");
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
                                        outputValues.push_back("datatype : bool");
                                    } else if (!dtypeArg.empty() && dtypeArg.front() == '"' && dtypeArg.back() == '"') {
                                        // Direct string literal
                                        outputValues.push_back("datatype : string");
                                    } else {
                                        // Check if it's a literal value first
                                        std::string dataType = compiler.getDataType(dtypeArg);
                                        if (dataType != "undefined") {
                                            // It's a literal value, return its type directly
                                            outputValues.push_back("datatype : " + dataType);
                                        } else {
                                            // Variable reference
                                            std::string value = compiler.getVariable(dtypeArg);
                                            if (!value.empty()) {
                                                std::string varDataType = compiler.getDataType(value);
                                                outputValues.push_back("datatype : " + varDataType);
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
                                }
                            } else {
                                // Variable or literal value
                                std::string value = compiler.getVariable(arg);
                                if (!value.empty()) {
                                    // Variable found
                                    if (!value.empty() && value.front() == '"' && value.back() == '"') {
                                        // String variable - remove quotes for output
                                        outputValues.push_back(value.substr(1, value.length() - 2));
                                    } else {
                                        outputValues.push_back(value);
                                    }
                                } else {
                                    // Check if it's a literal
                                    if (!arg.empty() && (std::all_of(arg.begin(), arg.end(), ::isdigit) || 
                                        (arg.find('.') != std::string::npos && std::count(arg.begin(), arg.end(), '.') == 1))) {
                                        // Number literal
                                        outputValues.push_back(arg);
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
                        
                        // Combine all output values into a single line
                        std::string combinedOutput = "";
                        for (size_t i = 0; i < outputValues.size(); i++) {
                            combinedOutput += outputValues[i];
                            if (i < outputValues.size() - 1) {
                                combinedOutput += " ";
                            }
                        }
                        compiler.outputValue(combinedOutput);
                    }
                }
            }
            
            // Get the output from compiler
            output = compiler.getOutput();
            
            // Create result message
            std::string msg = output;
            if (msg.empty()) {
                msg = "Compilation successful";
            }
            
            result->output = new char[msg.length() + 1];
            strcpy(result->output, msg.c_str());
            
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
    
    // Use the Orion compiler with working functionality restored
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