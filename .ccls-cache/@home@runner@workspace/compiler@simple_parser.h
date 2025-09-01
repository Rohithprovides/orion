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
        // Simple variable assignment: name = value
        if (check(TokenType::IDENTIFIER)) {
            size_t lookahead = current + 1;
            if (lookahead < tokens.size() && tokens[lookahead].type == TokenType::ASSIGN) {
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
        // Function call: name(args)
        if (check(TokenType::IDENTIFIER)) {
            size_t lookahead = current + 1;
            if (lookahead < tokens.size() && tokens[lookahead].type == TokenType::LPAREN) {
                std::string funcName = advance().value;
                advance(); // consume '('
                
                auto call = std::make_unique<FunctionCall>(funcName);
                
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
                
                return std::move(call);
            } else {
                // Variable reference
                std::string varName = advance().value;
                return std::make_unique<Identifier>(varName);
            }
        }
        
        return parsePrimary();
    }
    
    std::unique_ptr<Expression> parsePrimary() {
        if (check(TokenType::INTEGER)) {
            int value = std::stoi(advance().value);
            return std::make_unique<IntLiteral>(value);
        }
        
        if (check(TokenType::STRING)) {
            std::string value = advance().value;
            return std::make_unique<StringLiteral>(value);
        }
        
        if (check(TokenType::TRUE) || check(TokenType::FALSE)) {
            bool value = (advance().value == "True" || advance().value == "true");
            return std::make_unique<BoolLiteral>(value);
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