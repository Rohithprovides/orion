#include "ast.h"
#include <unordered_map>
#include <string>
#include <iostream>

namespace orion {

class TypeChecker : public ASTVisitor {
private:
    std::unordered_map<std::string, Type> variables;
    std::unordered_map<std::string, FunctionDeclaration*> functions;
    std::unordered_map<std::string, StructDeclaration*> structs;
    std::unordered_map<std::string, EnumDeclaration*> enums;
    
    Type currentReturnType;
    std::vector<std::string> errors;
    
public:
    bool check(Program& program) {
        errors.clear();
        
        // First pass: collect function, struct, and enum declarations
        for (auto& stmt : program.statements) {
            if (auto func = dynamic_cast<FunctionDeclaration*>(stmt.get())) {
                functions[func->name] = func;
            } else if (auto structDecl = dynamic_cast<StructDeclaration*>(stmt.get())) {
                structs[structDecl->name] = structDecl;
            } else if (auto enumDecl = dynamic_cast<EnumDeclaration*>(stmt.get())) {
                enums[enumDecl->name] = enumDecl;
            }
        }
        
        // Second pass: type check everything
        program.accept(*this);
        
        if (!errors.empty()) {
            std::cerr << "Type checking errors:\n";
            for (const auto& error : errors) {
                std::cerr << "  " << error << "\n";
            }
            return false;
        }
        
        return true;
    }
    
    const std::vector<std::string>& getErrors() const {
        return errors;
    }
    
private:
    void addError(const std::string& message) {
        errors.push_back(message);
    }
    
    Type inferType(Expression& expr) {
        // Simple type inference
        if (auto intLit = dynamic_cast<IntLiteral*>(&expr)) {
            return Type(TypeKind::INT32);
        }
        if (auto floatLit = dynamic_cast<FloatLiteral*>(&expr)) {
            return Type(TypeKind::FLOAT32);
        }
        if (auto stringLit = dynamic_cast<StringLiteral*>(&expr)) {
            return Type(TypeKind::STRING);
        }
        if (auto boolLit = dynamic_cast<BoolLiteral*>(&expr)) {
            return Type(TypeKind::BOOL);
        }
        if (auto id = dynamic_cast<Identifier*>(&expr)) {
            auto it = variables.find(id->name);
            if (it != variables.end()) {
                return it->second;
            }
            addError("Undefined variable: " + id->name);
            return Type(TypeKind::UNKNOWN);
        }
        if (auto binExpr = dynamic_cast<BinaryExpression*>(&expr)) {
            return inferBinaryType(*binExpr);
        }
        if (auto call = dynamic_cast<FunctionCall*>(&expr)) {
            auto it = functions.find(call->name);
            if (it != functions.end()) {
                return it->second->returnType;
            }
            addError("Undefined function: " + call->name);
            return Type(TypeKind::UNKNOWN);
        }
        
        return Type(TypeKind::UNKNOWN);
    }
    
    Type inferBinaryType(BinaryExpression& expr) {
        Type leftType = inferType(*expr.left);
        Type rightType = inferType(*expr.right);
        
        // Arithmetic operations
        if (expr.op == BinaryOp::ADD || expr.op == BinaryOp::SUB ||
            expr.op == BinaryOp::MUL || expr.op == BinaryOp::DIV || expr.op == BinaryOp::MOD) {
            
            if (leftType.kind == TypeKind::STRING || rightType.kind == TypeKind::STRING) {
                if (expr.op == BinaryOp::ADD) {
                    return Type(TypeKind::STRING); // String concatenation
                } else {
                    addError("Invalid operation on string");
                    return Type(TypeKind::UNKNOWN);
                }
            }
            
            if (leftType.kind == TypeKind::FLOAT32 || rightType.kind == TypeKind::FLOAT32 ||
                leftType.kind == TypeKind::FLOAT64 || rightType.kind == TypeKind::FLOAT64) {
                return Type(TypeKind::FLOAT32);
            }
            
            if (leftType.kind == TypeKind::INT32 || leftType.kind == TypeKind::INT64 ||
                rightType.kind == TypeKind::INT32 || rightType.kind == TypeKind::INT64) {
                return Type(TypeKind::INT32);
            }
            
            addError("Invalid types for arithmetic operation");
            return Type(TypeKind::UNKNOWN);
        }
        
        // Comparison operations
        if (expr.op == BinaryOp::EQ || expr.op == BinaryOp::NE ||
            expr.op == BinaryOp::LT || expr.op == BinaryOp::LE ||
            expr.op == BinaryOp::GT || expr.op == BinaryOp::GE) {
            return Type(TypeKind::BOOL);
        }
        
        // Logical operations
        if (expr.op == BinaryOp::AND || expr.op == BinaryOp::OR) {
            if (leftType.kind != TypeKind::BOOL || rightType.kind != TypeKind::BOOL) {
                addError("Logical operations require boolean operands");
            }
            return Type(TypeKind::BOOL);
        }
        
        return Type(TypeKind::UNKNOWN);
    }
    
    bool isCompatible(const Type& expected, const Type& actual) {
        if (expected.kind == TypeKind::UNKNOWN || actual.kind == TypeKind::UNKNOWN) {
            return true; // Allow unknown types (for error recovery)
        }
        
        if (expected.kind == actual.kind) {
            return true;
        }
        
        // Allow some implicit conversions
        if ((expected.kind == TypeKind::FLOAT32 || expected.kind == TypeKind::FLOAT64) &&
            (actual.kind == TypeKind::INT32 || actual.kind == TypeKind::INT64)) {
            return true; // int to float
        }
        
        if (expected.kind == TypeKind::INT64 && actual.kind == TypeKind::INT32) {
            return true; // int32 to int64
        }
        
        return false;
    }
    
public:
    void visit(IntLiteral& node) override {}
    void visit(FloatLiteral& node) override {}
    void visit(StringLiteral& node) override {}
    void visit(BoolLiteral& node) override {}
    void visit(Identifier& node) override {
        if (variables.find(node.name) == variables.end()) {
            addError("Undefined variable: " + node.name);
        }
    }
    
    void visit(BinaryExpression& node) override {
        node.left->accept(*this);
        node.right->accept(*this);
        inferBinaryType(node);
    }
    
    void visit(UnaryExpression& node) override {
        node.operand->accept(*this);
    }
    
    void visit(FunctionCall& node) override {
        auto it = functions.find(node.name);
        if (it == functions.end()) {
            addError("Undefined function: " + node.name);
            return;
        }
        
        FunctionDeclaration* func = it->second;
        
        // Check argument count
        if (node.arguments.size() != func->parameters.size()) {
            addError("Function " + node.name + " expects " + 
                    std::to_string(func->parameters.size()) + " arguments, got " +
                    std::to_string(node.arguments.size()));
            return;
        }
        
        // Check argument types
        for (size_t i = 0; i < node.arguments.size(); i++) {
            node.arguments[i]->accept(*this);
            Type argType = inferType(*node.arguments[i]);
            Type paramType = func->parameters[i].type;
            
            if (!isCompatible(paramType, argType)) {
                addError("Argument " + std::to_string(i + 1) + " to function " + node.name +
                        " has wrong type: expected " + paramType.toString() + 
                        ", got " + argType.toString());
            }
        }
    }
    
    void visit(VariableDeclaration& node) override {
        if (node.initializer) {
            node.initializer->accept(*this);
            
            Type initType = inferType(*node.initializer);
            
            if (!node.hasExplicitType) {
                // Type inference
                if (initType.kind == TypeKind::UNKNOWN) {
                    // Default to int32 if we can't infer
                    node.type = Type(TypeKind::INT32);
                } else {
                    node.type = initType;
                }
            } else {
                // Type checking
                if (!isCompatible(node.type, initType)) {
                    addError("Cannot assign " + initType.toString() + 
                            " to variable of type " + node.type.toString());
                }
            }
        } else if (!node.hasExplicitType) {
            addError("Variable " + node.name + " needs either explicit type or initializer");
        }
        
        variables[node.name] = node.type;
    }
    
    void visit(FunctionDeclaration& node) override {
        // Set up function context
        Type savedReturnType = currentReturnType;
        currentReturnType = node.returnType;
        
        // Add parameters to scope
        std::unordered_map<std::string, Type> savedVars = variables;
        for (const auto& param : node.parameters) {
            variables[param.name] = param.type;
        }
        
        if (node.isSingleExpression) {
            // Single expression function
            node.expression->accept(*this);
            Type exprType = inferType(*node.expression);
            
            if (!isCompatible(node.returnType, exprType)) {
                addError("Function " + node.name + " returns " + exprType.toString() +
                        " but declared return type is " + node.returnType.toString());
            }
        } else {
            // Block function
            for (auto& stmt : node.body) {
                stmt->accept(*this);
            }
        }
        
        // Restore context
        variables = savedVars;
        currentReturnType = savedReturnType;
    }
    
    void visit(BlockStatement& node) override {
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
    }
    
    void visit(ExpressionStatement& node) override {
        node.expression->accept(*this);
    }
    
    void visit(ReturnStatement& node) override {
        if (node.value) {
            node.value->accept(*this);
            Type returnType = inferType(*node.value);
            
            if (!isCompatible(currentReturnType, returnType)) {
                addError("Return type mismatch: expected " + currentReturnType.toString() +
                        ", got " + returnType.toString());
            }
        } else if (currentReturnType.kind != TypeKind::VOID) {
            addError("Non-void function must return a value");
        }
    }
    
    void visit(IfStatement& node) override {
        node.condition->accept(*this);
        Type condType = inferType(*node.condition);
        
        if (condType.kind != TypeKind::BOOL) {
            addError("If condition must be boolean, got " + condType.toString());
        }
        
        node.thenBranch->accept(*this);
        if (node.elseBranch) {
            node.elseBranch->accept(*this);
        }
    }
    
    void visit(WhileStatement& node) override {
        node.condition->accept(*this);
        Type condType = inferType(*node.condition);
        
        if (condType.kind != TypeKind::BOOL) {
            addError("While condition must be boolean, got " + condType.toString());
        }
        
        node.body->accept(*this);
    }
    
    void visit(ForStatement& node) override {
        if (node.init) {
            node.init->accept(*this);
        }
        
        if (node.condition) {
            node.condition->accept(*this);
            Type condType = inferType(*node.condition);
            
            if (condType.kind != TypeKind::BOOL) {
                addError("For condition must be boolean, got " + condType.toString());
            }
        }
        
        if (node.update) {
            node.update->accept(*this);
        }
        
        node.body->accept(*this);
    }
    
    void visit(StructDeclaration& node) override {
        // Basic validation - check for duplicate fields
        std::unordered_map<std::string, bool> fieldNames;
        for (const auto& field : node.fields) {
            if (fieldNames[field.name]) {
                addError("Duplicate field name in struct " + node.name + ": " + field.name);
            }
            fieldNames[field.name] = true;
        }
    }
    
    void visit(EnumDeclaration& node) override {
        // Basic validation - check for duplicate values
        std::unordered_map<std::string, bool> valueNames;
        for (const auto& value : node.values) {
            if (valueNames[value.name]) {
                addError("Duplicate value name in enum " + node.name + ": " + value.name);
            }
            valueNames[value.name] = true;
        }
    }
    
    void visit(Program& node) override {
        for (auto& stmt : node.statements) {
            stmt->accept(*this);
        }
    }
};

} // namespace orion
