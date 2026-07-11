#ifndef MLIRGEN_H
#define MLIRGEN_H

#include <memory>

namespace mlir {
class MLIRContext;
template <typename OpTY>
class OwningOpRef;
class ModuleOp;
} // namespace mlir


namespace toy {
class ModuleAST;

// Emit IR for the given AST Toy moduleAST, 
// return a newly created MLIR module.
mlir::OwningOpRef<mlir::ModuleOp> mlirGen(mlir::MLIRContext &context, ModuleAST &moduleAST);
} // namespace toy

#endif // MLIRGEN_H
