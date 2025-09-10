#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace orion {

// Forward declarations
class ASTVisitor;

// Base AST node class
class ASTNode {
public:
    int line = 0;
    int column = 0;
    
    ASTNode(int l = 0, int c = 0) : line(l), column(c) {}
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
    virtual std::string toString(int indent = 0) const = 0;
};

// Expression base class
class Expression : public ASTNode {
public:
    Expression(int line = 0, int column = 0) : ASTNode(line, column) {}
    virtual ~Expression() = default;
};

// Statement base class
class Statement : public ASTNode {
public:
    Statement(int line = 0, int column = 0) : ASTNode(line, column) {}
    virtual ~Statement() = default;
};

// Type representation
enum class TypeKind {
    INT32,
    INT64,
    FLOAT32,
    FLOAT64,
    BOOL,
    STRING,
    VOID,
    STRUCT,
    ENUM,
    FUNCTION,
    UNKNOWN
};

struct Type {
    TypeKind kind;
    std::string name;
    
    Type(TypeKind k = TypeKind::UNKNOWN, const std::string& n = "") 
        : kind(k), name(n) {}
    
    std::string toString() const {
        switch (kind) {
            case TypeKind::INT32: return "int";
            case TypeKind::INT64: return "int64";
            case TypeKind::FLOAT32: return "float";
            case TypeKind::FLOAT64: return "float64";
            case TypeKind::BOOL: return "bool";
            case TypeKind::STRING: return "string";
            case TypeKind::VOID: return "void";
            case TypeKind::STRUCT: return "struct " + name;
            case TypeKind::ENUM: return "enum " + name;
            default: return "unknown";
        }
    }
};

// Literals
class IntLiteral : public Expression {
public:
    int32_t value;
    
    IntLiteral(int32_t val, int line = 0, int column = 0) : Expression(line, column), value(val) {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override {
        return std::string(indent, ' ') + "IntLiteral(" + std::to_string(value) + ")";
    }
};

class FloatLiteral : public Expression {
public:
    double value;
    
    FloatLiteral(double val, int line = 0, int column = 0) : Expression(line, column), value(val) {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override {
        return std::string(indent, ' ') + "FloatLiteral(" + std::to_string(value) + ")";
    }
};

class StringLiteral : public Expression {
public:
    std::string value;
    
    StringLiteral(const std::string& val, int line = 0, int column = 0) : Expression(line, column), value(val) {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override {
        return std::string(indent, ' ') + "StringLiteral(\"" + value + "\")";
    }
};

class BoolLiteral : public Expression {
public:
    bool value;
    
    BoolLiteral(bool val, int line = 0, int column = 0) : Expression(line, column), value(val) {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override {
        return std::string(indent, ' ') + "BoolLiteral(" + (value ? "True" : "False") + ")";
    }
};

// Identifier
class Identifier : public Expression {
public:
    std::string name;
    
    Identifier(const std::string& n, int line = 0, int column = 0) : Expression(line, column), name(n) {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override {
        return std::string(indent, ' ') + "Identifier(" + name + ")";
    }
};

// Binary operations
enum class BinaryOp {
    ADD, SUB, MUL, DIV, MOD,
    POWER, FLOOR_DIV,
    EQ, NE, LT, LE, GT, GE,
    AND, OR,
    ASSIGN
};

class BinaryExpression : public Expression {
public:
    std::unique_ptr<Expression> left;
    BinaryOp op;
    std::unique_ptr<Expression> right;
    
    BinaryExpression(std::unique_ptr<Expression> l, BinaryOp o, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Unary operations
enum class UnaryOp {
    PLUS, MINUS, NOT
};

class UnaryExpression : public Expression {
public:
    UnaryOp op;
    std::unique_ptr<Expression> operand;
    
    UnaryExpression(UnaryOp o, std::unique_ptr<Expression> expr)
        : op(o), operand(std::move(expr)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Function call
class FunctionCall : public Expression {
public:
    std::string name;
    std::vector<std::unique_ptr<Expression>> arguments;
    
    FunctionCall(const std::string& n) : name(n) {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Tuple expression
class TupleExpression : public Expression {
public:
    std::vector<std::unique_ptr<Expression>> elements;
    
    TupleExpression() {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Variable declaration
class VariableDeclaration : public Statement {
public:
    std::string name;
    Type type;
    std::unique_ptr<Expression> initializer;
    bool hasExplicitType;
    bool isConstant;
    
    VariableDeclaration(const std::string& n, const Type& t, std::unique_ptr<Expression> init, bool explicit_type = false, bool constant = false)
        : name(n), type(t), initializer(std::move(init)), hasExplicitType(explicit_type), isConstant(constant) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Function parameter
struct Parameter {
    std::string name;
    Type type;
    
    Parameter(const std::string& n, const Type& t) : name(n), type(t) {}
};

// Function declaration
class FunctionDeclaration : public Statement {
public:
    std::string name;
    std::vector<Parameter> parameters;
    Type returnType;
    std::vector<std::unique_ptr<Statement>> body;
    bool isSingleExpression;
    std::unique_ptr<Expression> expression; // for single-expression functions
    
    FunctionDeclaration(const std::string& n, const Type& ret)
        : name(n), returnType(ret), isSingleExpression(false) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Block statement
class BlockStatement : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Expression statement
class ExpressionStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;
    
    ExpressionStatement(std::unique_ptr<Expression> expr)
        : expression(std::move(expr)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override {
        return std::string(indent, ' ') + "ExpressionStatement:\n" + 
               expression->toString(indent + 2);
    }
};

// Tuple assignment for (a,b) = (c,d) syntax
class TupleAssignment : public Statement {
public:
    std::vector<std::unique_ptr<Expression>> targets;  // left side (a,b)
    std::vector<std::unique_ptr<Expression>> values;   // right side (c,d)
    
    TupleAssignment() {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Chain assignment for a=b=5 syntax
class ChainAssignment : public Statement {
public:
    std::vector<std::string> variables;  // variables to assign to (a, b)
    std::unique_ptr<Expression> value;   // value to assign (5)
    
    ChainAssignment() {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Global statement
class GlobalStatement : public Statement {
public:
    std::vector<std::string> variables;  // global x, y, z
    
    GlobalStatement() {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Local statement
class LocalStatement : public Statement {
public:
    std::vector<std::string> variables;  // local x, y, z
    
    LocalStatement() {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Return statement
class ReturnStatement : public Statement {
public:
    std::unique_ptr<Expression> value;
    
    ReturnStatement(std::unique_ptr<Expression> val = nullptr)
        : value(std::move(val)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// If statement
class IfStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenBranch;
    std::unique_ptr<Statement> elseBranch;
    
    IfStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> then_stmt)
        : condition(std::move(cond)), thenBranch(std::move(then_stmt)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// While statement
class WhileStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
    
    WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> body_stmt)
        : condition(std::move(cond)), body(std::move(body_stmt)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// For statement
class ForStatement : public Statement {
public:
    std::unique_ptr<Statement> init;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> update;
    std::unique_ptr<Statement> body;
    
    ForStatement(std::unique_ptr<Statement> init_stmt,
                 std::unique_ptr<Expression> cond,
                 std::unique_ptr<Expression> upd,
                 std::unique_ptr<Statement> body_stmt)
        : init(std::move(init_stmt)), condition(std::move(cond)), 
          update(std::move(upd)), body(std::move(body_stmt)) {}
    
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Struct field
struct StructField {
    std::string name;
    Type type;
    
    StructField(const std::string& n, const Type& t) : name(n), type(t) {}
};

// Struct declaration
class StructDeclaration : public Statement {
public:
    std::string name;
    std::vector<StructField> fields;
    
    StructDeclaration(const std::string& n) : name(n) {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Enum value
struct EnumValue {
    std::string name;
    int value;
    
    EnumValue(const std::string& n, int v) : name(n), value(v) {}
};

// Enum declaration
class EnumDeclaration : public Statement {
public:
    std::string name;
    std::vector<EnumValue> values;
    
    EnumDeclaration(const std::string& n) : name(n) {}
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Program (root node)
class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    
    void accept(ASTVisitor& visitor) override;
    std::string toString(int indent = 0) const override;
};

// Visitor pattern for AST traversal
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    virtual void visit(IntLiteral& node) = 0;
    virtual void visit(FloatLiteral& node) = 0;
    virtual void visit(StringLiteral& node) = 0;
    virtual void visit(BoolLiteral& node) = 0;
    virtual void visit(Identifier& node) = 0;
    virtual void visit(BinaryExpression& node) = 0;
    virtual void visit(UnaryExpression& node) = 0;
    virtual void visit(FunctionCall& node) = 0;
    virtual void visit(TupleExpression& node) = 0;
    virtual void visit(VariableDeclaration& node) = 0;
    virtual void visit(FunctionDeclaration& node) = 0;
    virtual void visit(BlockStatement& node) = 0;
    virtual void visit(ExpressionStatement& node) = 0;
    virtual void visit(TupleAssignment& node) = 0;
    virtual void visit(ChainAssignment& node) = 0;
    virtual void visit(GlobalStatement& node) = 0;
    virtual void visit(LocalStatement& node) = 0;
    virtual void visit(ReturnStatement& node) = 0;
    virtual void visit(IfStatement& node) = 0;
    virtual void visit(WhileStatement& node) = 0;
    virtual void visit(ForStatement& node) = 0;
    virtual void visit(StructDeclaration& node) = 0;
    virtual void visit(EnumDeclaration& node) = 0;
    virtual void visit(Program& node) = 0;
};

} // namespace orion

#endif // AST_H
