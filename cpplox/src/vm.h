#ifndef CPPLOX_VM_H_
#define CPPLOX_VM_H_

#include "stack.h"
#include "string.h"
#include "table.h"
#include "value.h"
namespace lox {
    class Chunk;
    class Binary;

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
        void negate();
        void runtimeError(StringView sv) const;
        void verifyNumber(size_t stackIndex = 0) const;
        void verifyString(size_t stackIndex = 0) const;
        void defineGlobal(const Chunk& chunk, uint32_t constant);
        InterpretResult pushGlobal(const Chunk& chunk, uint32_t constant);
        InterpretResult assignGlobal(const Chunk& chunk, uint32_t constant);

        void pushLocal(size_t constant);
        void assignLocal(size_t constant);

        double popNumber();
        void binaryOp(const Binary& bin);
        DynamicStack<Value> stack;
        Table<InternedString, Value> globals;
    };
}
#endif