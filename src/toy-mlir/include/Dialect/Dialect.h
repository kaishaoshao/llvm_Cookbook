#ifndef DIALECT_H
#define DIALECT_H 

#include <mlir/Bytecode/BytecodeOpInterface.h>
#include <mlir/IR/Dialect.h>
#include <mlir/IR/SymbolTable.h>
#include <mlir/Interfaces/CallInterfaces.h>
#include <mlir/Interfaces/FunctionInterfaces.h>
#include <mlir/Interfaces/SideEffectInterfaces.h>

// toy dialect
#include "Dialect/Dialect.h.inc"

// toy op
#define GET_OP_CLASSES
#include "Dialect/Ops.h.inc"


#endif // DIALECT_H
