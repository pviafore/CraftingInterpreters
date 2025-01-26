#ifndef CPPLOX_VM_H_
#define CPPLOX_VM_H_

#include "stack.h"
#include "value.h"
namespace lox {
    class Chunk;

    enum class InterpretResult {
        Ok,
        CompileError,
        RuntimeError
    };
    class VM {
    public:
        InterpretResult interpret(const Chunk& chunk);
        InterpretResult run(const Chunk& chunk);

        bool diagnosticMode = false;

    private:
        Stack<Value, 256> stack;
    };
}
#endif