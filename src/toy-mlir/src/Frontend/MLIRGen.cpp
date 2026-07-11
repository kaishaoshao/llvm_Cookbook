
// 它的核心作用是：把 Toy 语言 Parser 生成的 AST 抽象语法树，
// 转换成 MLIR 中的 Toy Dialect IR。
// 它处在 Toy 编译流程的中间阶段：Parser 已经把源码解析成 AST，
// 这个文件负责把 AST 逐步翻译成 toy.func、toy.constant、
// toy.add、toy.reshape、toy.print 等 MLIR Operation。
// Toy 源码
//   ↓ Lexer
// Token 流
//   ↓ Parser
// Toy AST
//   ↓ MLIRGen.cpp
// Toy Dialect MLIR
//   ↓ 后续 Pass / Lowering
// Affine / LLVM / 机器代码

#include "AST.h"
#include "Lexer.h"
#include "MLIRGen.h"
#include "Dialect.h"

#include <mlir/IR/Block.h>
#include <mlir/IR/Value.h>
#include <mlir/IR/Diagnostics.h>
#include <mlir/Support/LogicalResult.h>

#include <mlir/IR/Builders.h>
#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/BuiltinTypes.h>
#include <mlir/IR/MLIRContext.h>
#include <mlir/IR/Verifier.h>

#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/ScopedHashTable.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Twine.h>

#include <cassert>
#include <cstdint>
#include <functional>
#include <numeric>
#include <optional>
#include <vector>

using namespace mlir::toy;
using namespace toy;

using llvm::SmallVector;
using llvm::ArrayRef;
using llvm::StringRef;
using llvm::dyn_cast;
using llvm::cast;
using llvm::isa;
using llvm::Twine;
using llvm::ScopedHashTableScope;


namespace {

class MLIRGenImpl {
public:
    // 构招函数，创建一个MLIRGenImpl对象，需要传入一个MLIRContext
    // 然后利用context初始化，也就是初始化MLIR的IR Builder
    MLIRGenImpl(mlir::MLIRContext &context) : builder(&context) {}

    mlir::ModuleOp mlirGen(ModuleAST &moduleAST) {
        // 创建一个空的MLIR module, getUnknownLoc()表示没有位置信息
        // ==> module {}
        theModule = mlir::ModuleOp::create(builder.getUnknownLoc());
        
        for(FunctionAST &function : moduleAST) 
           mlirGen(function);
        
        if(failed(mlir::verify(theModule))) {
            theModule.emitError("module verification error");
            return nullptr;
        }

        return theModule;
    }

private:
    mlir::ModuleOp theModule;

    mlir::OpBuilder builder;

    llvm::ScopedHashTable<StringRef, mlir::Value> symbolTable;

    mlir::Location loc(const Location &loc) {
        return mlir::FileLineColLoc::get(builder.getStringAttr(*loc.file),
                                         loc.line, loc.col);
    }

    mlir::LogicalResult declare(StringRef var, mlir::Value value) {
        if(symbolTable.count(var))
            return mlir::failure();
        symbolTable.insert(var, value);
        return mlir::success();
    }

    mlir::toy::FuncOp mlirGen(PrototypeAST &proto) {
        auto location = loc(proto.loc());

        llvm::SmallVector<mlir::Type, 4> argTypes(proto.getArgs().size(),
                                                  getType(VarType()));
        auto funcType = builder.getFunctionType(argTypes, std::nullopt);
        return builder.create<mlir::toy::FuncOp>(location, proto.getName(),
                                                 funcType);
    }

    mlir::toy::FuncOp mlirGen(FunctionAST &funcAST) {
        ScopedHashTableScope<StringRef, mlir::Value> varScope(symbolTale);

        builder.setInsertionPointToEnd(theModule.getBody());
        mlir::toy::FuncOp function= mlirGen(*funcAST.getProto());
        if(!function)
            return nullptr;
        
        mlir::Block &entryBlock = function.front();
        auto protoArgs = funcAST.getProto()->getArgs();

        for(const auto nameValue : 
            llvm::zip(protoArgs, entryBlock.getArguments())) {
               if(failed(declare(std::get<0>(nameValue)->getName(), std::get<1>(nameValue)))) {
                  return nullptr;
               }
            }
            
        builder.setInsertionPointToStart(&entryBlock);

        if(mlir::failed(mlirGen(*funcAST.getBody()))) {
            function.erase();
            return nullptr;
        }

        ReturnOp returnOp;
        if(!entryBlock.empty())
          returnOp = dyn_cast<ReturnOp>(entryBlock.back());
        if(!returnOp) {
            builder.create<ReturnOp>(loc(funcAST.getProto()->loc()));
        } else if(returnOp.hasOperand()) {
            function.setType(builder.getFunctionType(
                function.getFunctionType().getInputs(), getType(VarType{})
            ));
        }
        return function;
    }

    mlir::Value mlirGen(BinaryExprAST &binop) {
        mlir::Value lhs = mlirGen(*binop.getLHS());
        if(!lhs)
            return nullptr;
        mlir::Value rhs = mlirGen(*binop.getRHS());
        if(!rhs)
            return nullptr;
        auto location = loc(binop.getLoc());

        switch(binop.getOp()) {
            case '+' :
                return builder.create<AddOp>(location, lhs, rhs);
            case '*' : 
                return builder.create<MulOp>(location, lhs, rhs);>
        }

        emitError(location, "invalid binary operator '") << binop.getOp() << "'";   
        return nullptr;
    }

    mlir::Value mlirGen(VariableExprAST &var) {
        auto *init = vardecl.getInitVal();
        if(!init) {
            emitError(loc(vardecl.loc()), "missing initializer in variable declaration");
            return nullptr;
        }    
        
        mlir::Value value = mlirGen(*init);
        if(!value)
            return nullptr;

        if(!vardecl.getType().shape.empty()) {
            value = builder.create<ReshapeOp>(loc(vardecl.loc()),
                getType(vardecl.getType(), value));
        }

        if(failed(declare(vardecl.getName(), value)))
            return nullptr;
        return value;
    }

    mlir::LogicalResult mlirGen(ExprASTList &blockAST) {
        ScopedHashTableScope<StringRef, mlir::Value> varScope(symbolTable);
        for(auto &expr : blockAST) {
            if(auto *vardecl = dyn_cast<VarDeclExprAST>(expr.get())) {
                if(!mlirGen(*vardecl))
                    return mlir::failure();
                continue;
            }
            if(auto *ret = dyn_cast<ReturnExprAST>(expr.get())) 
                return mlirGen(*ret);
            // TODO: mlir::failed() ?
            if(auto *print = dyn_cast<PrintExprAST>(expr.get())) {
                if(mlir::failed(mlirGen(*print)))
                    return mlir::success();
                continue;
            }

            if(!mlirGen(*expr))
                return mlir::failure();

        }
        return mlir::success();
    }

    mlir::Type getType(ArrayRef<int64_t> shape) {
        if(shape.empty())
            return mlir::UnrankedTensorType::get(builder.getF64Type());
        return mlir::RankedTensorType::get(shape, builder.getF64Type());
    }
   
    mlir::Type getType(const VarType &type) {
        return getType(type.shape);
    }
};
} // namespace

namespace toy {
    // The public API for codegen
    mlir::OwningOpRef<mlir::ModuleOP> mlirGen(mlir::MLIRContext &context,
                                              ModuleAST &moduleAST) {
        return MLIRGenImpl(context).mlirGen(moduleAST);
    }
} // namespace toy
