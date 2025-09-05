#ifndef SIMPLE_PARSER_H
#define SIMPLE_PARSER_H

#include "ast.h"
#include "lexer.h"
#include <memory>
#include <vector>
#include <stdexcept>

namespace orion {

class SimpleOrionParser {
private:
    std::vector<Token> tokens;
    size_t current;
    
public:
    SimpleOrionParser(const std::vector<Token>& toks) : tokens(toks), current(0) {}
    
    std::unique_ptr<Program> parse() {
        auto program = std::make_unique<Program>();
        
        while (!isAtEnd()) {
            if (peek().type == TokenType::NEWLINE) {
                advance();
                continue;
            }
            
            auto stmt = parseStatement();
            if (stmt) {
                program->statements.push_back(std::move(stmt));
            }
        }
        
        return program;
    }
    
private:
    bool isAtEnd() const {
        return current >= tokens.size() || peek().type == TokenType::EOF_TOKEN;
    }
    
    const Token& peek() const {
        if (current >= tokens.size()) {
            static Token eof(TokenType::EOF_TOKEN, "", 0, 0);
            return eof;
        }
        return tokens[current];
    }
    
    Token advance() {
        if (!isAtEnd()) current++;
        return tokens[current - 1];
    }
    
    bool check(TokenType type) const {
        if (isAtEnd()) return false;
        return peek().type == type;
    }
    
    std::unique_ptr<Statement> parseStatement() {
        // Check for tuple assignment first
        if (check(TokenType::LPAREN)) {
            return parseTupleAssignmentOrExpression();
        }
        
        // Global statement
        if (check(TokenType::GLOBAL)) {
            return parseGlobalStatement();
        }
        
        // Local statement
        if (check(TokenType::LOCAL)) {
            return parseLocalStatement();
        }
        
        // Function declaration (fn name() { ... })
        if (check(TokenType::IDENTIFIER) && tokens[current].value == "fn") {
            return parseFunctionDeclaration();
        }
        
        // Variable declaration or expression
        return parseVariableDeclarationOrExpression();
    }
    
    std::unique_ptr<FunctionDeclaration> parseFunctionDeclaration() {
        advance(); // consume 'fn'
        
        if (!check(TokenType::IDENTIFIER)) {
            throw std::runtime_error("Expected function name");
        }
        
        std::string funcName = advance().value;
        auto func = std::make_unique<FunctionDeclaration>(funcName, Type(TypeKind::VOID));
        
        if (!check(TokenType::LPAREN)) {
            throw std::runtime_error("Expected '(' after function name");
        }
        advance(); // consume '('
        
        if (!check(TokenType::RPAREN)) {
            throw std::runtime_error("Parameters not supported yet");
        }
        advance(); // consume ')'
        
        if (!check(TokenType::LBRACE)) {
            throw std::runtime_error("Expected '{' for function body");
        }
        advance(); // consume '{'
        
        // Parse function body
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            if (check(TokenType::NEWLINE)) {
                advance();
                continue;
            }
            
            auto stmt = parseStatement();
            if (stmt) {
                func->body.push_back(std::move(stmt));
            }
        }
        
        if (!check(TokenType::RBRACE)) {
            throw std::runtime_error("Expected '}' after function body");
        }
        advance(); // consume '}'
        
        return func;
    }
    
    std::unique_ptr<GlobalStatement> parseGlobalStatement() {
        advance(); // consume 'global'
        
        auto globalStmt = std::make_unique<GlobalStatement>();
        
        // Parse comma-separated variable names
        if (!check(TokenType::IDENTIFIER)) {
            throw std::runtime_error("Expected variable name after 'global'");
        }
        
        do {
            if (!check(TokenType::IDENTIFIER)) {
                throw std::runtime_error("Expected identifier in global statement");
            }
            globalStmt->variables.push_back(advance().value);
        } while (check(TokenType::COMMA) && (advance(), true));
        
        return globalStmt;
    }
    
    std::unique_ptr<LocalStatement> parseLocalStatement() {
        advance(); // consume 'local'
        
        auto localStmt = std::make_unique<LocalStatement>();
        
        // Parse comma-separated variable names
        if (!check(TokenType::IDENTIFIER)) {
            throw std::runtime_error("Expected variable name after 'local'");
        }
        
        do {
            if (!check(TokenType::IDENTIFIER)) {
                throw std::runtime_error("Expected identifier in local statement");
            }
            localStmt->variables.push_back(advance().value);
        } while (check(TokenType::COMMA) && (advance(), true));
        
        return localStmt;
    }
    
    std::unique_ptr<Statement> parseTupleAssignmentOrExpression() {
        // Parse what looks like a tuple
        auto tupleExpr = parseExpression();
        
        // Check if it's followed by assignment
        if (check(TokenType::ASSIGN)) {
            advance(); // consume '='
            
            // This is a tuple assignment
            auto assignment = std::make_unique<TupleAssignment>();
            
            // Extract targets from the tuple expression
            if (auto tuple = dynamic_cast<TupleExpression*>(tupleExpr.get())) {
                // Move elements from tuple to assignment targets
                for (auto& element : tuple->elements) {
                    assignment->targets.push_back(std::move(element));
                }
                tupleExpr.release(); // We've moved the contents
            } else {
                // Single target (not actually a tuple)
                assignment->targets.push_back(std::move(tupleExpr));
            }
            
            // Parse right side - could be a tuple or single expression
            auto rightExpr = parseExpression();
            if (auto rightTuple = dynamic_cast<TupleExpression*>(rightExpr.get())) {
                // Move elements from right tuple to assignment values
                for (auto& element : rightTuple->elements) {
                    assignment->values.push_back(std::move(element));
                }
                rightExpr.release(); // We've moved the contents
            } else {
                // Single value
                assignment->values.push_back(std::move(rightExpr));
            }
            
            return std::move(assignment);
        } else {
            // Not an assignment, just a regular expression statement
            return std::make_unique<ExpressionStatement>(std::move(tupleExpr));
        }
    }

    std::unique_ptr<Statement> parseVariableDeclarationOrExpression() {
        // Check for chain assignment: a=b=5
        if (check(TokenType::IDENTIFIER)) {
            // Scan ahead to detect chain assignment pattern
            std::vector<size_t> assignPositions;
            size_t lookahead = current;
            
            while (lookahead < tokens.size()) {
                if (tokens[lookahead].type == TokenType::ASSIGN) {
                    assignPositions.push_back(lookahead);
                } else if (tokens[lookahead].type == TokenType::NEWLINE || 
                          tokens[lookahead].type == TokenType::SEMICOLON ||
                          tokens[lookahead].type == TokenType::EOF_TOKEN) {
                    break;
                }
                lookahead++;
            }
            
            if (assignPositions.size() > 1) {
                // Chain assignment detected (a=b=5)
                auto chainAssign = std::make_unique<ChainAssignment>();
                
                // Parse variables: for "a=b=5", we need [a, b]
                size_t pos = current;
                for (size_t i = 0; i < assignPositions.size(); i++) {
                    size_t assignPos = assignPositions[i];
                    // Get the identifier before this assignment
                    if (pos < assignPos && tokens[pos].type == TokenType::IDENTIFIER) {
                        chainAssign->variables.push_back(tokens[pos].value);
                    }
                    // Move to the position after this = sign for next variable
                    pos = assignPos + 1;
                }
                
                // Move current to the last assignment position + 1 (the value)
                current = assignPositions.back() + 1;
                chainAssign->value = parseExpression();
                
                return std::move(chainAssign);
            } else if (assignPositions.size() == 1) {
                // Simple variable assignment: name = value
                std::string varName = advance().value;
                advance(); // consume '='
                
                auto init = parseExpression();
                return std::make_unique<VariableDeclaration>(varName, Type(), std::move(init), false);
            }
        }
        
        // Expression statement
        auto expr = parseExpression();
        return std::make_unique<ExpressionStatement>(std::move(expr));
    }
    
    std::unique_ptr<Expression> parseExpression() {
        return parseLogicalOr();
    }
    
    std::unique_ptr<Expression> parseLogicalOr() {
        auto expr = parseLogicalAnd();
        
        while (check(TokenType::OR)) {
            advance(); // consume '||'
            auto right = parseLogicalAnd();
            expr = std::make_unique<BinaryExpression>(std::move(expr), BinaryOp::OR, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<Expression> parseLogicalAnd() {
        auto expr = parseEquality();
        
        while (check(TokenType::AND)) {
            advance(); // consume '&&'
            auto right = parseEquality();
            expr = std::make_unique<BinaryExpression>(std::move(expr), BinaryOp::AND, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<Expression> parseEquality() {
        auto expr = parseComparison();
        
        while (check(TokenType::EQ) || check(TokenType::NE)) {
            BinaryOp op = (peek().type == TokenType::EQ) ? BinaryOp::EQ : BinaryOp::NE;
            advance();
            auto right = parseComparison();
            expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<Expression> parseComparison() {
        auto expr = parseTerm();
        
        while (check(TokenType::LT) || check(TokenType::LE) || check(TokenType::GT) || check(TokenType::GE)) {
            BinaryOp op;
            switch (peek().type) {
                case TokenType::LT: op = BinaryOp::LT; break;
                case TokenType::LE: op = BinaryOp::LE; break;
                case TokenType::GT: op = BinaryOp::GT; break;
                case TokenType::GE: op = BinaryOp::GE; break;
                default: throw std::runtime_error("Invalid comparison operator");
            }
            advance();
            auto right = parseTerm();
            expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<Expression> parseTerm() {
        auto expr = parseFactor();
        
        while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
            BinaryOp op = (peek().type == TokenType::PLUS) ? BinaryOp::ADD : BinaryOp::SUB;
            advance();
            auto right = parseFactor();
            expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<Expression> parseFactor() {
        auto expr = parsePower();
        
        while (check(TokenType::MULTIPLY) || check(TokenType::DIVIDE) || check(TokenType::MODULO) || check(TokenType::FLOOR_DIVIDE)) {
            BinaryOp op;
            switch (peek().type) {
                case TokenType::MULTIPLY: op = BinaryOp::MUL; break;
                case TokenType::DIVIDE: op = BinaryOp::DIV; break;
                case TokenType::MODULO: op = BinaryOp::MOD; break;
                case TokenType::FLOOR_DIVIDE: op = BinaryOp::FLOOR_DIV; break;
                default: throw std::runtime_error("Invalid factor operator");
            }
            advance();
            auto right = parsePower();
            expr = std::make_unique<BinaryExpression>(std::move(expr), op, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<Expression> parsePower() {
        auto expr = parseUnary();
        
        // Exponentiation is right-associative
        if (check(TokenType::POWER)) {
            advance(); // consume '**'
            auto right = parsePower(); // Right-associative: a**b**c = a**(b**c)
            expr = std::make_unique<BinaryExpression>(std::move(expr), BinaryOp::POWER, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<Expression> parseUnary() {
        if (check(TokenType::NOT) || check(TokenType::MINUS) || check(TokenType::PLUS)) {
            UnaryOp op;
            switch (peek().type) {
                case TokenType::NOT: op = UnaryOp::NOT; break;
                case TokenType::MINUS: op = UnaryOp::MINUS; break;
                case TokenType::PLUS: op = UnaryOp::PLUS; break;
                default: throw std::runtime_error("Invalid unary operator");
            }
            advance();
            auto right = parseUnary();
            return std::make_unique<UnaryExpression>(op, std::move(right));
        }
        
        return parseCall();
    }
    
    std::unique_ptr<Expression> parseCall() {
        auto expr = parsePrimary();
        
        while (true) {
            if (check(TokenType::LPAREN)) {
                // Function call
                if (auto id = dynamic_cast<Identifier*>(expr.get())) {
                    advance(); // consume '('
                    auto call = std::make_unique<FunctionCall>(id->name);
                    expr.release(); // Release ownership since we're replacing it
                    
                    // Parse arguments
                    if (!check(TokenType::RPAREN)) {
                        do {
                            call->arguments.push_back(parseExpression());
                        } while (check(TokenType::COMMA) && (advance(), true));
                    }
                    
                    if (!check(TokenType::RPAREN)) {
                        throw std::runtime_error("Expected ')' after function arguments");
                    }
                    advance(); // consume ')'
                    expr = std::move(call);
                } else {
                    throw std::runtime_error("Invalid function call");
                }
            } else {
                break;
            }
        }
        
        return expr;
    }
    
    std::unique_ptr<Expression> parsePrimary() {
        if (check(TokenType::INTEGER)) {
            Token token = advance();
            int value = std::stoi(token.value);
            return std::make_unique<IntLiteral>(value, token.line, token.column);
        }
        
        if (check(TokenType::STRING)) {
            Token token = advance();
            return std::make_unique<StringLiteral>(token.value, token.line, token.column);
        }
        
        if (check(TokenType::TRUE) || check(TokenType::FALSE)) {
            Token token = advance();
            bool value = (token.value == "True");
            return std::make_unique<BoolLiteral>(value, token.line, token.column);
        }
        
        if (check(TokenType::LPAREN)) {
            advance(); // consume '('
            
            // Check if this is a tuple or just a parenthesized expression
            auto firstExpr = parseExpression();
            
            if (check(TokenType::COMMA)) {
                // This is a tuple
                auto tuple = std::make_unique<TupleExpression>();
                tuple->elements.push_back(std::move(firstExpr));
                
                do {
                    advance(); // consume ','
                    tuple->elements.push_back(parseExpression());
                } while (check(TokenType::COMMA));
                
                if (!check(TokenType::RPAREN)) {
                    throw std::runtime_error("Expected ')' after tuple");
                }
                advance(); // consume ')'
                return std::move(tuple);
            } else {
                // Just a parenthesized expression
                if (!check(TokenType::RPAREN)) {
                    throw std::runtime_error("Expected ')' after expression");
                }
                advance(); // consume ')'
                return firstExpr;
            }
        }
        
        if (check(TokenType::IDENTIFIER)) {
            std::string varName = advance().value;
            return std::make_unique<Identifier>(varName);
        }
        
        throw std::runtime_error("Unexpected token in expression");
    }
};

} // namespace orion

#endif // SIMPLE_PARSER_H