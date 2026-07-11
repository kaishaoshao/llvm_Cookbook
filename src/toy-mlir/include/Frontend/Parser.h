#ifndef __TOY_PASER_H__
#define __TOY_PASER_H__

//===----------------------------------------------------------------------===//
// 该文件实现Toy语言的解析器，处理Lexer提供的Token流并生成AST。
//===----------------------------------------------------------------------===//

#include "Lexer.h"
#include "AST.h"

namespace toy {

/// 梯度下降解析器：将Token流转换为AST (不进行语义检查，仅语法分析)
class Parser {
public:
    Parser(Lexer &lexer) : lexer(lexer) {} // 构造函数，传入Lexer引用

    /// 解析整个模块(一组函数定义)
    std::unique_ptr<ModuleAST> parseModule();

private:

    Lexer &lexer;

    std::unique_ptr<ReturnExprAST> parseReturn();

    std::unique_ptr<ExprAST> parseExpr();

    std::unique_ptr<ExprAST> parseNumberExpr();

    std::unique_ptr<ExprAST> parseTensorLiteralExpr();

    std::unique_ptr<ExprAST> parseParenExpr();

    std::unique_ptr<ExprAST> parseIdentifierExpr();

    std::unique_ptr<ExprAST> parsePrimary();

    std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs);

    std::unique_ptr<ExprAST> parseExpression();

    std::unique_ptr<VarType> parseType();

    std::unique_ptr<VarDeclExprAST> parseDeclaration();

    std::unique_ptr<ExprASTList> parseBlock();

    std::unique_ptr<PrototypeAST> parsePrototype();

    std::unique_ptr<FunctionAST> parseDefinition();

    int getTokPrecedence();

    template <typename R, typename T, typename U = const char *>
    std::unique_ptr<R> parseError(T &&expected, U &&context = " ");
};

} // namespace toy

#endif //  __TOY_PASER_H__