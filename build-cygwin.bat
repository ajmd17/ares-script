@echo off

rem Compile AVM library
echo Compiling avm library...
g++ -shared -o bin/avm.dll -std=gnu++11 -O2 -w -Iinclude/ -Iinclude/avm/ src/avm/arraylist.cpp src/avm/avm.cpp src/avm/byte_stream.cpp src/avm/frame.cpp src/avm/function.cpp src/avm/heap.cpp src/avm/object.cpp src/avm/reference.cpp src/avm/variable.cpp src/avm/vm_state.cpp src/avm/check_args.cpp

rem Compile the ARES compiler
echo Compiling ARES compiler...
g++ -shared -o bin/alang.dll -std=gnu++11 -w -Iinclude/ -Iinclude/compiler/ src/compiler/abytecodegen.cpp src/compiler/acompiler.cpp src/compiler/alexer.cpp src/compiler/aparser.cpp src/compiler/error.cpp src/compiler/semantic.cpp src/compiler/token.cpp

rem Compile the executable
echo Compiling ARES executable...
g++ -o bin/ares.exe -std=gnu++11 -w -Iinclude/ -Iinclude/ares/ -Iinclude/compiler/ -Iinclude/avm/ src/ares/ascript.cpp src/ares/rtlib.cpp src/ares/main.cpp -Lbin/ -lavm -lalang