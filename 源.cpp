#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <cctype>
#include "expr.h"

enum class TokenType {
    NUMBER,    // 数字
    OPERATOR,  // 运算符
    LPAREN,    // 左括号 (
    RPAREN     // 右括号 )
};

// Token结构体：存储类型和值
struct Token {
    TokenType type;
    std::string value;

    Token(TokenType t, const std::string& v) : type(t), value(v) {}
};

// 获取运算符优先级：优先级越高数值越大
int getPriority(const std::string& op) {
    if (op == "+" || op == "-") return 1;
    if (op == "*" || op == "/" || op == "%") return 2;
    if (op == "^" || op == "**") return 3;
    return 0; // 括号优先级为0
}

// 判断是否是左结合运算符（幂运算右结合，其余左结合）
bool isLeftAssociative(const std::string& op) {
    if (op == "^" || op == "**") return false;
    return true;
}

// 词法分析：将表达式拆分为Token列表
std::vector<Token> tokenize(const std::string& expr) {
    std::vector<Token> tokens;
    std::string numBuffer; // 存储数字字符
    int len = expr.length();

    for (int i = 0; i < len; ++i) {
        char c = expr[i];

        // 跳过空格
        if (isspace(c)) continue;

        // 数字或小数点
        if (isdigit(c) || c == '.') {
            numBuffer += c;
            // 检查下一个字符是否还是数字/小数点，不是则结束当前数字
            if (i + 1 >= len || !isdigit(expr[i + 1]) && expr[i + 1] != '.') {
                tokens.emplace_back(TokenType::NUMBER, numBuffer);
                numBuffer.clear();
            }
        }
        // 运算符：+ - * / % ^ (注意处理正负号前缀)
        else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^') {
            // 处理正负号前缀（出现在开头、左括号后、运算符后）
            if ((c == '+' || c == '-') && (i == 0 || expr[i - 1] == '(' ||
                expr[i - 1] == '+' || expr[i - 1] == '-' || expr[i - 1] == '*' ||
                expr[i - 1] == '/' || expr[i - 1] == '%' || expr[i - 1] == '^')) {
                numBuffer += c; // 把符号加入数字缓冲区（如-5、+3）
            }
            // 处理**（幂运算的另一种写法）
            else if (c == '*' && i + 1 < len && expr[i + 1] == '*') {
                tokens.emplace_back(TokenType::OPERATOR, "**");
                i++; // 跳过下一个*
            }
            // 普通运算符
            else {
                tokens.emplace_back(TokenType::OPERATOR, std::string(1, c));
            }
        }
        // 括号
        else if (c == '(') {
            tokens.emplace_back(TokenType::LPAREN, "(");
        }
        else if (c == ')') {
            tokens.emplace_back(TokenType::RPAREN, ")");
        }
        // 非法字符
        else {
            throw std::invalid_argument("错误：包含非法字符 '" + std::string(1, c) + "'");
        }
    }

    return tokens;
}

// 中缀表达式转后缀表达式（Shunting-yard算法）
std::vector<Token> infixToPostfix(const std::vector<Token>& tokens) {
    std::vector<Token> postfix;
    std::stack<Token> opStack;
    int parenCount = 0; // 括号计数器，校验括号匹配

    for (const auto& token : tokens) {
        // 数字直接加入后缀列表
        if (token.type == TokenType::NUMBER) {
            postfix.push_back(token);
        }
        // 左括号入栈
        else if (token.type == TokenType::LPAREN) {
            opStack.push(token);
            parenCount++;
        }
        // 右括号：弹出栈内运算符直到左括号
        else if (token.type == TokenType::RPAREN) {
            parenCount--;
            if (parenCount < 0) {
                throw std::invalid_argument("错误：右括号多于左括号");
            }
            while (!opStack.empty() && opStack.top().type != TokenType::LPAREN) {
                postfix.push_back(opStack.top());
                opStack.pop();
            }
            if (opStack.empty()) {
                throw std::invalid_argument("错误：括号不匹配（缺少左括号）");
            }
            opStack.pop(); // 弹出左括号（不加入后缀列表）
        }
        // 运算符：处理优先级和结合性
        else if (token.type == TokenType::OPERATOR) {
            const std::string& currOp = token.value;
            while (!opStack.empty() && opStack.top().type == TokenType::OPERATOR) {
                const std::string& topOp = opStack.top().value;
                int currPri = getPriority(currOp);
                int topPri = getPriority(topOp);

                // 左结合：当前优先级 <= 栈顶优先级 则弹出
                // 右结合：当前优先级 < 栈顶优先级 则弹出
                if ((isLeftAssociative(currOp) && currPri <= topPri) ||
                    (!isLeftAssociative(currOp) && currPri < topPri)) {
                    postfix.push_back(opStack.top());
                    opStack.pop();
                }
                else {
                    break;
                }
            }
            opStack.push(token);
        }
    }

    // 校验括号是否匹配
    if (parenCount != 0) {
        throw std::invalid_argument("错误：括号不匹配（缺少右括号）");
    }

    // 弹出栈内剩余运算符
    while (!opStack.empty()) {
        if (opStack.top().type == TokenType::LPAREN) {
            throw std::invalid_argument("错误：括号不匹配（缺少右括号）");
        }
        postfix.push_back(opStack.top());
        opStack.pop();
    }

    return postfix;
}

// 计算两个数的运算结果
double calculate(double a, double b, const std::string& op) {
    if (op == "+") return a + b;
    if (op == "-") return a - b;
    if (op == "*") return a * b;
    if (op == "/") {
        if (b == 0) {
            throw std::invalid_argument("错误：除数不能为0");
        }
        return a / b;
    }
    if (op == "%") {
        // 取模运算只支持整数（这里做简单转换，也可支持浮点数取模）
        if (b == 0) {
            throw std::invalid_argument("错误：取模除数不能为0");
        }
        return fmod(a, b);
    }
    if (op == "^" || op == "**") {
        return pow(a, b);
    }
    throw std::invalid_argument("错误：未知运算符 '" + op + "'");
}

// 后缀表达式求值
double evaluatePostfix(const std::vector<Token>& postfix) {
    std::stack<double> numStack;

    for (const auto& token : postfix) {
        if (token.type == TokenType::NUMBER) {
            // 转换字符串为数字
            double num = 0.0;
            try {
                num = std::stod(token.value);
            }
            catch (...) {
                throw std::invalid_argument("错误：数字格式非法 '" + token.value + "'");
            }
            numStack.push(num);
        }
        else if (token.type == TokenType::OPERATOR) {
            // 运算符需要至少两个操作数
            if (numStack.size() < 2) {
                throw std::invalid_argument("错误：表达式格式错误（运算符缺少操作数）");
            }
            double b = numStack.top(); numStack.pop();
            double a = numStack.top(); numStack.pop();
            double res = calculate(a, b, token.value);
            numStack.push(res);
        }
    }

    // 最终栈内只能有一个结果
    if (numStack.size() != 1) {
        throw std::invalid_argument("错误：表达式格式错误（操作数多余）");
    }

    return numStack.top();
}

// 主计算函数：整合所有步骤
double calculateExpression(const std::string& expr) {
    try {
        // 1. 词法分析
        std::vector<Token> tokens = tokenize(expr);
        // 2. 中缀转后缀
        std::vector<Token> postfix = infixToPostfix(tokens);
        // 3. 求值
        return evaluatePostfix(postfix);
    }
    catch (const std::invalid_argument& e) {
        throw; // 抛出错误让主函数处理
    }
}

int main() {
    std::cout << "=====数学表达式计算器=====" << std::endl;
    std::cout << "支持：+ - * / % ^/** 括号 () 正负号前缀" << std::endl;
    std::cout << "输入 'quit' 退出程序" << std::endl;
    std::cout << "================================" << std::endl;

    std::string expr;
    while (true) {
        std::cout << "\n请输入表达式：";
        std::getline(std::cin, expr);
        
        if (expr == "quit") {
            std::cout << "程序退出！" << std::endl;
            break;                                                      // 退出程序
        }

        if (expr.empty()) continue;                                     // 空输入跳过

        try {
            double result = calculateExpression(expr);
            std::cout << "计算结果：" << result << std::endl;
        }
        catch (const std::invalid_argument& e) {
            std::cout << e.what() << std::endl;
        }
    }

    return 0;
}