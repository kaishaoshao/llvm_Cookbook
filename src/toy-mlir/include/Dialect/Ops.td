#ifndef TOY_OPS
#define TOY_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

// 创建一个叫 toy 的 MLIR Dialect。以后 IR 里面出现的操作都会带这个前缀，比如：
// toy.constant
// toy.add
// toy.mul
// toy.func
// toy.return
// toy.print
def Toy_Dialect : Dialect { 
	let name = "toy";
	let cppNamespace = "::mlir::toy";
}

// 这个相当于给所有 Toy 操作做一个统一基类模板。
// 比如：
// def AddOp : Toy_Op<"add">
// 最终表示的 MLIR 操作名字就是：
// toy.add
// 因为：
// Dialect 名称 toy + mnemonic add = toy.add
// 所以这个文件里每一个：
// def XxxOp : Toy_Op<"...">
// 都会生成一个 Toy 方言下的操作。
// TODO: Trait 的作用？
class Toy_Op<string mnemonic, list<Trait> traits = []> :
	Op<Toy_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Toy Operations
//===----------------------------------------------------------------------===//

// FuncOp
def FuncOp : Toy_Op<"func", [FunctionOpInterface, IsolatedFromAbove]> {
	let summary = "user defined function operation";

	let description = [{
		The "toy.func" operation represents a user defined function. These are
    callable SSA-region operations that contain toy computations.

    Example:
    ```mlir
    toy.func @main() {
      %0 = toy.constant dense<5.500000e+00> : tensor<f64>
      %1 = toy.reshape(%0 : tensor<f64>) to tensor<2x2xf64>
      toy.print %1 : tensor<2x2xf64>
      toy.return
    }
    ```
	}];

	let arguments = (ins
		SymbolNameAttr: $sym_name,
		TypeAttrOf<FunctionType>: $function_type,
		OptionalAttr<DictArrayAttr>: $arg_attr,
		OptionalAttr<DictArrayAttr>: $res_attr
	);

	let regions = (region AnyRegion:$body);

	let builders = [
		OpBuilder<(ins "StringRef":$name,
		"FunctionType":$type,
		CArg<"ArrayRef<NamedAttribute>", "{}">:$attrs)
		>];

	let extraClassDeclaration = [{
	    ArrayRef<Type> getArgumentTypes() {
			return getFunctionType().getInputs();
		}

		ArrayRef<Type> getResultTypes() {
            return	getFunctionType().getResults();
		}
	}];

	let hasCustomAssemblyFormat = 1;
	let skipDefaultBuilders = 1;
}

// GenericCallOp
def GenericCallOp : Toy_Op<"generic_call"> {
	let summary = "generic call operation";
	let description = [{
		Generic calls represent calls to a user defined function that needs to
    	be specialized for the shape of its arguments. The callee name is attached
    	as a symbol reference via an attribute. The arguments list must match the
    	arguments expected by the callee. For example:
    	```mlir
    	 %4 = toy.generic_call @my_func(%1, %3)
           : (tensor<2x3xf64>, tensor<2x3xf64>) -> tensor<*xf64>
    	```
    	This is only valid if a function named "my_func" exists and takes two
    	arguments.
	}];

	let arguments = (ins FlatSymbolRefAttr: $callee, Variadic<F64Tensor>:$inputs);

	let results = (outs F64Tensor);

	let assemblyFormat = [{
		$callee `(` $inputs `)` attr-dict `:` functional-type($inputs, results)
	}];

	let builders = [
		OpBuilder<(ins "StringRef": $callee, "ArrayRef<Value>":$argumenrs)>
	];
}

// ReturnOP
def ReturnOp: Toy_Op<"return", [Pure, HasParent<"FuncOp">, Terminator]> {
	let summary = "return operation";
	let description = [{
		The "return" operation represents a return operation within a function.
		The operation takes an optional ternsor operand and produces no result.
		The operand type must match the signature of the function that contains
    	the operation. For example:
    	```mlir
      	toy.func @foo() -> tensor<2xf64> {
        	...
        	toy.return %0 : tensor<2xf64>
      	}
    	```
	}];
	let arguments = (ins Variadic<F64Tensor>:$input);

    // TODO:
	let assemblyFormat = "($input^ `:` type($input)) ? attr-dict";

	let builders = [
		OpBuilder<(ins), [{ build($_builder, $_state, std::nullopt); }]>
	];

	let extraClassDeclaration = [{
		bool hasOperand() {
			return getNumOperands() != 0;
		}
	}];

	let hasVerifier = 1;
}

// ConstantOp 用来把一个字面量常量变成 MLIR SSA value
// [Pure]表示这个操作无副作用。
// 
def ConstantOp : Toy_Op<"constant", [Pure]> {
	let summary = "constant";
	// 常量的值不是普通输入 operand，而是作为 attribute 存在 operation 里面。
	let description = [{
		Constant operation turns a literal into an SSA value. The data is attached
    to the operation as an attribute. For example:
	  ```
		%0 = toy.constant dense<[[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]>
			  : ternsor<2x3xf64>
		```	
	}];

	let arguments = (ins F64ElementsAttr: $value);

	let results = (outs F64Tensor);

	let hasCustomAssemblyFormat = 1;

	let builders = [
		OpBuilder<(ins "DenseElementsAttr": $value), [{
			build($_builder, $_state, value.getType(), value);
			}]>,
		
		OpBuilder<(ins "double":$value)>
	];

	let hasVerifier = 1;
}

// PrintOp
def PrintOp : Toy_Op<"print"> {
  let summary = "print operation";
  let description = [{
    The "print" builtin operation prints a given input tensor, and produces
    no results.
  }];
  let arguments = (ins F64Tensor: $input);

  let assemblyFormat = "$input attr-dict `:` type($input)";
}

def AddOp : Toy_Op<"add"> {
	let summary = "element-wise addition aperation";

  let description = [{
    The "add" operation performs element-wise addition between two tensors.
    The shapes of the tensor operands are expected to match.
  }];

	let arguments = (ins F64Tensor: $lhs, F64Tensor:$rhs);
	
	let results = (outs F64Tensor);

	let hasCustomAssemblyFormat = 1;

	let builders = [
		OpBuilder<(ins "Value":$lhs, "Value":$rhs)>
	];
} 

// MulOp
def MulOp : Toy_Op<"mul"> {
	let summary = "element-wise multiplication operation";
	let description = [{
		The "mul" operation performs element-wise multiplication between two tensors.
		The shapes of the tensor operands are expected to match.
	}];
	let arguments = (ins F64Tensor: $lhs, F64Tensor:$rhs);
	let results = (outs F64Tensor);

	let hasCustomAssemblyFormat = 1;

	let builders = [
		OpBuilder<(ins "Value":$lhs, "Value":$rhs)>
	];
}

// ReshapeOp
def ReshapeOp : Toy_Op<"reshape"> {
	let summary = "tensor reshape operation";
	let description = [{
	    Reshape operation is transforming its input tensor into a new tensor with
    	the same number of elements but different shapes. For example:

    	```mlir
	   	%0 = toy.reshape (%arg1 : tensor<10xf64>) to tensor<5x2xf64>
    	```
	}];

	let arguments = (ins F64Tensor: $input);
	let results = (outs StaticShapeTensorOf<[F64]>);

	let assemblyFormat = [{
	  `(` $input `:` type($input) `)` attr-dict `to` type(results)
	}];
}


// TransposeOp
def TransposeOp : Toy_Op<"transpose"> {
	let summary = "transpose operation";

	let arguments = (ins F64Tensor: $input);
	let results = (outs F64Tensor);

	let assemblyFormat = [{
		`(` $input `:` type($input) `)` attr-dict `to` type(results)
	}];

	let builders = [
		OpBuilder<(ins "Value":$input)>
	];

	let hasVerifier = 1;

}

#endif // TOY_OPS
