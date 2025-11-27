#ifndef CPPLOX_VM_H_
#define CPPLOX_VM_H_

#include "chunk.h"
#include "object.h"
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
        VM();
        InterpretResult interpret(const String& string);
        InterpretResult run();

        bool diagnosticMode = false;

        struct CallFrame {
        public:
            CallFrame(SharedPtr<Function> f, DynamicStack<Value>& stack, int argCount = 0) : function(f), instructionPtr(function->getChunk()->begin()), slots(&stack), offset(argCount + 1) {}
            Chunk::InstructionIterator& getIp() {
                return instructionPtr;
            }

            void push(Value v) {
                (*slots).push(v);
            }

            const Value& peek(size_t index = 0) const {
                return (*slots).peek(index - offset);
            }

            void assign(size_t index, const Value& value) {
                (*slots)[index - offset] = value;
            }

            SharedPtr<Function> getFunction() const {
                return function;
            }

        private:
            SharedPtr<Function> function;
            Chunk::InstructionIterator instructionPtr;
            DynamicStack<Value>* slots;
            size_t offset;
        };

    private:
        void negate();
        void runtimeError(StringView sv) const;
        void verifyNumber(size_t stackIndex = 0) const;
        void verifyString(size_t stackIndex = 0) const;
        void defineGlobal(const Chunk& chunk, uint32_t constant);
        void defineNative(StringView name, NativeFunction::Func f, size_t argCount);

        InterpretResult pushGlobal(const Chunk& chunk, uint32_t constant);
        InterpretResult assignGlobal(const Chunk& chunk, uint32_t constant);

        void pushLocal(size_t constant);
        void assignLocal(size_t constant);
        void callValue(Value callee, int argCount);
        void call(SharedPtr<Function> func, int argCount);
        double popNumber();
        void binaryOp(const Binary& bin);
        DynamicStack<Value> stack;
        DynamicStack<CallFrame> frames;
        Table<InternedString, Value> globals;
    };
}
#endif