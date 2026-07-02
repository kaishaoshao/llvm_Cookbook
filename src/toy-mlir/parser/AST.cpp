//===----------------------------------------------------------------------===//
//
// 该文件实现了Toy语言的抽象语法树(AST)转储功能，用于调试和可视化AST结构。
//
//===----------------------------------------------------------------------===//

#include "../include/AST.h"

#include <llvm/ADT/Twine.h>
#include <llvm/ADT/TypeSwitch.h>
#include <llvm/Support/raw_ostream.h>

using namespace toy;

namespace {

// 缩进管理辅助类 (RAII模式)
struct Indent {
  Indent(int &level) : level(level) { ++level; } // 构造时增加缩进层级
  ~Indent() { --level; }                         // 析构时减少缩进层级
  int &level;
};

/// AST转储器实现类，负责遍历AST并格式化输出节点信息
class ASTDumper {
public:
  void dump(ModuleAST *node); // 模块入口点

private:
  // 各类型节点的转储方法
  void dump(const VarType &type);
  void dump(VarDeclExprAST *varDecl);
  void dump(ExprAST *expr);
  void dump(ExprASTList *exprList);
  void dump(NumberExprAST *num);
  void dump(LiteralExprAST *node);
  void dump(VariableExprAST *node);
  void dump(ReturnExprAST *node);
  void dump(BinaryExprAST *node);
  void dump(CallExprAST *node);
  void dump(PrintExprAST *node);
  void dump(PrototypeAST *node);
  void dump(FunctionAST *node);

  // 缩进输出辅助方法
  void indent() {
    for (int i = 0; i < curIndent; i++)
      llvm::errs() << " "; // 每个缩进级别输出两个空格
  }

  int curIndent = 0; // 当前缩进级别
};

} // namespace

/// 生成节点位置信息的格式化字符串
template <typename T> 
static std::string loc(T *node) {
  const auto &loc = node->getLoc();
  return (llvm::Twine("@") + *loc.file + ":" + llvm::Twine(loc.line) + ":" +
          llvm::Twine(loc.col)).str();
}

// 缩进宏：进入作用域增加缩进，离开时自动恢复
#define INDENT()                                                               \
  Indent level_(curIndent); /* RAII对象管理缩进 */                             \
  indent();

/// 表达式动态分发处理：根据具体子类型调用对应的转储方法
void ASTDumper::dump(ExprAST *expr) {
  // 不理解用法
  llvm::TypeSwitch<ExprAST *>(expr)
      .Case<BinaryExprAST, CallExprAST, LiteralExprAST, NumberExprAST,
            PrintExprAST, ReturnExprAST, VarDeclExprAST, VariableExprAST>(
          [&](auto *node) { this->dump(node); }) // 类型匹配处理
      .Default([&](ExprAST *) {                  // 未知表达式类型的处理
        INDENT();
        llvm::errs() << "<未知表达式类型，种类 " << expr->getKind() << ">\n";
      });
}

// 变量声明处理： 输出变量名、类型和位置，然后处理初始化表达式
void ASTDumper::dump(VarDeclExprAST *varDecl) {
  INDENT();
  llvm::errs() << "VarDecl " << varDecl->getName();
  dump(varDecl->getType()); // 输出变量类型信息
  llvm::errs() << " " << loc(varDecl) << "\n";
  dump(varDecl->getInitVal()); // 递归处理初始化表达式
}

/// 代码块处理： 输出块内所有表达式
/// a "block", or a list of expression
void ASTDumper::dump(ExprASTList *exprList) {
  INDENT();
  llvm::errs() << "Block {\n";
  for (auto &expr : *exprList) {
    dump(expr.get());
  }
  llvm::errs() << "}\n";
}

/// 数字字面量处理： 直接输出数值和位置
/// A literal number, just print the value
void ASTDumper::dump(NumberExprAST *num) {
  INDENT();
  llvm::errs() << "numberExprAST: " << num->getValue() << " " << loc(num)
               << "\n";
}

/// Helper to print recursively a literal.
/// This handles nested array like
/// a = [1, 2, [3, 4]];
/// We print out such array with the dimensions
/// spelled out at every level
/// <2,2>[<2>[1,2].<2>[3,4]]
void printLitHelper(ExprAST *LotOrNum) {
  // Inside a literal expression, we can have either
  // a number or another literal
  if (auto *num = llvm::dyn_cast<NumberExprAST>(LotOrNum)) {
    llvm::errs() << num->getValue();
    return;
  }
  auto *literal = llvm::cast<LiteralExprAST>(LotOrNum);
  // Print the dimensions
  llvm::errs() << "<";
  llvm::interleaveComma(literal->getValues(), llvm::errs(),
                        [&](auto &elt) { printLitHelper(elt.get()); });
  llvm::errs() << " ]";
}

// 字面量处理： 直接输出字面量值和位置
/// Print a literal, see the recursive helper
/// above for the implementation
void ASTDumper::dump(LiteralExprAST *node) {
  INDENT();
  llvm::errs() << "Literal: ";
  printLitHelper(node);
  llvm::errs() << " " << loc(node) << "\n";
}

// 变量引用处理： 输出变量名和位置
/// Print a variable reference (just a name)
void ASTDumper::dump(VariableExprAST *node) {
  INDENT();
  llvm::errs() << "var: " << node->getName() << " " << loc(node) << "\n";
}

/// 模块处理： 输出模块内所有函数定义
/// Return statement print the return and its (optional) argument
void ASTDumper::dump(ReturnExprAST *node) {
  INDENT();
  llvm::errs() << "Return\n";
  if (node->getExpr().has_value()) {
    INDENT();
    llvm::errs() << "(void)\n";
  }
}

// 二元表达式处理： 输出操作符和位置
void ASTDumper::dump(BinaryExprAST *node) {
  INDENT();
  llvm::errs() << "BinOp: " << node->getOp() << " " << loc(node) << "\n";
  dump(node->getLHS()); // 递归处理左操作数
  dump(node->getRHS()); // 递归处理右操作数
}

// 函数调用处理： 输出函数名、参数列表和位置
/// Print a call expression, first the calleename
/// and the list of args by
/// recursing into each individual argument
void ASTDumper::dump(CallExprAST *node) {
  INDENT();
  llvm::errs() << "Call " << node->getCallee() << "'[ " << loc(node) << "\n";
  for (auto &arg : node->getArgs())
    dump(arg.get());
  indent();
  llvm::errs() << "]\n";
}

// 打印表达式处理： 输出打印内容和位置
/// Print a builtin print call, first the builtin
/// name and then the argument
void ASTDumper::dump(PrintExprAST *node) {
  INDENT();
  llvm::errs() << "Print [ " << loc(node) << "\n";
  dump(node->getArg());
  indent();
  llvm::errs() << "]\n";
}

// 变量类型处理： 输出变量类型信息
/// Print type: only the shape is printed in
/// between '<' and '>'
void ASTDumper::dump(const VarType &type) {
  llvm::errs() << "<";
  llvm::interleaveComma(type.shape, llvm::errs());
  llvm::errs() << ">";
}

/// Print a function prototype,first the function
/// name, and then the list of
/// parameters names.
// 函数原型处理： 输出函数名、参数列表和位置
void ASTDumper::dump(PrototypeAST *node) {
  INDENT();
  llvm::errs() << "Proto '" << node->getName() << "' " << loc(node)
               << " \n";
  indent();
  llvm::errs() << "Params: [";

  llvm::interleaveComma(node->getArgs(), llvm::errs(),
                        [](auto &arg) { llvm::errs() << arg->getName(); });
  llvm::errs() << "]\n";
}

//  函数定义处理： 输出函数名、参数列表和位置
/// Print a function, first the prototype and then the body.
void ASTDumper::dump(FunctionAST *node) {
  INDENT();
  llvm::errs() << "Functiion \n";
  dump(node->getProto());
  dump(node->getBody());
}

/// Print a module, actually loop over the
/// functions and print them in swquence.
void ASTDumper::dump(ModuleAST *node) {
  INDENT();
  llvm::errs() << "Module:\n";
  for (auto &f : *node)
    dump(&f);
}

namespace toy {
// Public API
void dump(ModuleAST &module) { ASTDumper().dump(&module); }
} // namespace toy