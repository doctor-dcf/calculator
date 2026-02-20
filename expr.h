#pragma once
#include<string>
#include<vector>
enum class TokenType;
struct Token;
int getPriority(const std::string& op);
bool isLeftAssociative(const std::string& op);
std::vector<Token> tokenize(const std::string& expr);
std::vector<Token> infixToPostfix(const std::vector<Token>& tokens);
double evaluatePostfix(const std::vector<Token>& postfix);
double calculate(double a, double b, const std::string& op);
double calculateExpression(const std::string& expr);