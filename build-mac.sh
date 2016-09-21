#!/bin/sh/

echo "Compiling AVM library..."
g++ -shared -o bin/libavm.dylib -std=gnu++11 -O2 -w -Iinclude/ -Iinclude/avm/ src/avm/arraylist.cpp src/avm/avm.cpp src/avm/byte_stream.cpp src/avm/frame.cpp src/avm/function.cpp src/avm/heap.cpp src/avm/object.cpp src/avm/reference.cpp src/avm/variable.cpp src/avm/vm_state.cpp src/avm/check_args.cpp

echo "Compiling the compiler library..."
g++ -shared -o bin/libalang.dylib -std=gnu++11 -w -Iinclude/ -Iinclude/compiler/ src/compiler/bytecode_generator.cpp src/compiler/compiler.cpp src/compiler/lexer.cpp src/compiler/parser.cpp src/compiler/error.cpp src/compiler/semantic.cpp src/compiler/token.cpp src/compiler/ast/ast_binary_op.cpp src/compiler/ast/ast_expression.cpp src/compiler/ast/ast_float.cpp src/compiler/ast/ast_integer.cpp src/compiler/ast/ast_node.cpp src/compiler/ast/ast_unary_op.cpp src/compiler/state.cpp

echo "Compiling and linking the executable..."
g++ -o bin/ares -std=gnu++11 -w -Iinclude/ -Iinclude/ares/ -Iinclude/compiler/ -Iinclude/avm/ src/ares/ascript.cpp src/ares/rtlib.cpp src/ares/main.cpp -Lbin/ -lavm -lalang
