#ifndef CPPLOX_VM_H_
#define CPPLOX_VM_H_

#include "stack.h"
#include "string.h"
#include "value.h"
namespace lox {
    class Chunk;

    enum class InterpretResult {
        Ok = 0,
        CompileError = 65,
        RuntimeError = 70
    };
    class VM {
    public:
        InterpretResult interpret(const String& string);
        InterpretResult run(const Chunk& chunk);

        bool diagnosticMode = false;

    private:
        DynamicStack<Value> stack;
    };
}
#endif