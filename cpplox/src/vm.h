#ifndef CPPLOX_VM_H_
#define CPPLOX_VM_H_

#include "chunk.h"
#include "list.h"
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
            CallFrame(Callable f, DynamicStack<Value>& stack, size_t offset = 0) : function(f), instructionPtr(lox::getFunction(f)->getChunk()->begin()), slots(&stack), offset(offset) {}
            Chunk::InstructionIterator& getIp() {
                return instructionPtr;
            }

            void push(Value v) {
                slots->push(v);
            }

            const Value& peek(size_t index = 0) const {
                return (*slots)[index + offset];
            }

            void assign(size_t index, const Value& value) {
                (*slots)[index + offset] = value;
            }

            Callable getCallable() const {
                return function;
            }

            SharedPtr<Function> getFunction() const {
                return lox::getFunction(function);
            }

            size_t getOffset() const {
                return offset;
            }

        private:
            Callable function;
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
        void defineMethod(InternedString name, bool isInitializer = false);
        SharedPtr<UpValueObj> captureUpValue(DynamicStack<Value>::iterator);
        void closeUpValues(const DynamicStack<Value>::iterator iter);
        void bindMethod(SharedPtr<Class> cls, InternedString name);

        InterpretResult pushGlobal(const Chunk& chunk, uint32_t constant);
        InterpretResult assignGlobal(const Chunk& chunk, uint32_t constant);

        void pushLocal(size_t constant);
        void assignLocal(size_t constant);
        void callValue(Value callee, int argCount);
        void call(Callable func, size_t argCount);
        void invoke(InternedString name, uint8_t argCount);
        void invokeFromClass(SharedPtr<Class> cls, InternedString name, uint8_t argCount);
        void innerInvoke(SharedPtr<Class> cls, InternedString superclassName, InternedString name, uint8_t argCount);
        double popNumber();
        void binaryOp(const Binary& bin);
        DynamicStack<Value> stack;
        DynamicStack<CallFrame> frames;
        List<SharedPtr<UpValueObj>> openUpValues;
        Table<InternedString, Value> globals;
    };
}
#endif