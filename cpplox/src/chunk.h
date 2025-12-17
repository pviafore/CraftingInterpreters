#ifndef CPPLOX_CHUNK_H_
#define CPPLOX_CHUNK_H_

#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <variant>

#include "common.h"
#include "table.h"
#include "value.h"
#include "vector.h"
namespace lox {
    enum class OpCode : uint8_t {
        Add,
        BitwiseAnd,
        BitwiseOr,
        Call,
        Class,
        Closure,
        CloseUpValue,
        Constant,
        DefineGlobal,
        LongDefineGlobal,
        Equal,
        GetGlobal,
        GetLocal,
        GetUpValue,
        LongGetGlobal,
        Invoke,
        Inherit,
        Method,
        Initializer,
        GetProperty,
        SetProperty,
        InnerInvoke,
        Greater,
        JumpIfFalse,
        Jump,
        Less,
        Nil,
        Not,
        True,
        False,
        Divide,
        LongConstant,
        Loop,
        Multiply,
        Negate,
        Print,
        Pop,
        Return,
        SetGlobal,
        SetLocal,
        SetUpValue,
        LongSetGlobal,
        Subtract,
        Unknown
    };

    class Chunk;
    struct _Instruction {
    public:
        _Instruction(OpCode opcode, size_t size, std::string name) : opcode(opcode), size(size), name(std::move(name)) {}
        OpCode opcode;
        size_t size = 1;
        std::string name;
    };

    template <typename T>
    size_t getValueSize();

    template <typename T>
    T readFromBuffer(const std::byte* buffer);

    template <typename T>
    class _OpAndValueInstruction : public _Instruction {
    public:
        _OpAndValueInstruction(const std::byte* buffer, OpCode opcode, std::string s) : _Instruction(opcode, getValueSize<T>(), s), constantAddress(readFromBuffer<T>(buffer + 1)) {}
        T value() const {
            return constantAddress;
        }

    private:
        T constantAddress;
    };

    class Call : public _OpAndValueInstruction<uint8_t> {
    public:
        Call(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::Call, "OP_CALL") {}
    };
    class ClassOp : public _OpAndValueInstruction<uint8_t> {
    public:
        ClassOp(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::Class, "OP_CLASS") {}
    };

    std::vector<Function::UpValue> getUpValues(const Chunk* chunk, size_t index);
    class ClosureOp : public _OpAndValueInstruction<uint8_t> {
    public:
        ClosureOp(const std::byte* buffer, const Chunk* chunk) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::Closure, "OP_CLOSURE"), upvalues(lox::getUpValues(chunk, value())) {
            size = 2 + 2 * upvalues.size();
        }

        const std::vector<Function::UpValue>& getUpValues() const {
            return upvalues;
        }

    private:
        std::vector<Function::UpValue> upvalues;
    };

    class Invoke : public _OpAndValueInstruction<uint8_t> {
    public:
        Invoke(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::Invoke, "OP_INVOKE"), argCount(uint8_t(*(buffer + 2))) {
            size = 3;
        }
        uint8_t getArgumentCount() const {
            return argCount;
        }

    private:
        uint8_t argCount;
    };
    class InnerInvoke : public _OpAndValueInstruction<uint8_t> {
    public:
        InnerInvoke(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::InnerInvoke, "OP_INNER_INVOKE"), methodName(uint8_t(*(buffer + 2))), argCount(uint8_t(*(buffer + 3))) {
            size = 4;
        }
        uint8_t getArgumentCount() const {
            return argCount;
        }
        uint8_t getMethodName() const {
            return methodName;
        }

    private:
        uint8_t methodName;
        uint8_t argCount;
    };

    class Constant : public _OpAndValueInstruction<uint8_t> {
    public:
        Constant(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::Constant, "OP_CONSTANT") {}
    };

    class MethodOp : public _OpAndValueInstruction<uint8_t> {
    public:
        MethodOp(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::Method, "OP_METHOD") {}
    };

    class Initializer : public _OpAndValueInstruction<uint8_t> {
    public:
        Initializer(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::Initializer, "OP_INITIALIZER") {}
    };
    uint32_t toAddress(const std::byte* buffer);
    class LongConstant : public _OpAndValueInstruction<uint32_t> {
    public:
        LongConstant(const std::byte* buffer) : _OpAndValueInstruction<uint32_t>(buffer, OpCode::Constant, "OP_LONG_CONSTANT") {}
    };
    class DefineGlobal : public _OpAndValueInstruction<uint8_t> {
    public:
        DefineGlobal(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::Constant, "OP_DEFINE_GLOBAL") {}
    };

    class LongDefineGlobal : public _OpAndValueInstruction<uint32_t> {
    public:
        LongDefineGlobal(const std::byte* buffer) : _OpAndValueInstruction<uint32_t>(buffer, OpCode::Constant, "OP_LONG_DEFINE_GLOBAL") {}
    };
    class GetGlobal : public _OpAndValueInstruction<uint8_t> {
    public:
        GetGlobal(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::Constant, "OP_GET_GLOBAL") {}
    };

    class LongGetGlobal : public _OpAndValueInstruction<uint32_t> {
    public:
        LongGetGlobal(const std::byte* buffer) : _OpAndValueInstruction<uint32_t>(buffer, OpCode::Constant, "OP_LONG_GET_GLOBAL") {}
    };
    class SetGlobal : public _OpAndValueInstruction<uint8_t> {
    public:
        SetGlobal(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::Constant, "OP_SET_GLOBAL") {}
    };

    class LongSetGlobal : public _OpAndValueInstruction<uint32_t> {
    public:
        LongSetGlobal(const std::byte* buffer) : _OpAndValueInstruction<uint32_t>(buffer, OpCode::Constant, "OP_LONG_SET_GLOBAL") {}
    };

    class GetLocal : public _OpAndValueInstruction<uint8_t> {
    public:
        GetLocal(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::GetLocal, "OP_GET_LOCAL") {}
    };
    class SetLocal : public _OpAndValueInstruction<uint8_t> {
    public:
        SetLocal(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::SetLocal, "OP_SET_LOCAL") {}
    };
    class GetProperty : public _OpAndValueInstruction<uint8_t> {
    public:
        GetProperty(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::GetProperty, "OP_GET_PROPERTY") {}
    };
    class SetProperty : public _OpAndValueInstruction<uint8_t> {
    public:
        SetProperty(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::SetProperty, "OP_SET_PROPERTY") {}
    };
    class GetUpValue : public _OpAndValueInstruction<uint8_t> {
    public:
        GetUpValue(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::GetUpValue, "OP_GET_UPVALUE") {}
    };
    class SetUpValue : public _OpAndValueInstruction<uint8_t> {
    public:
        SetUpValue(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::SetUpValue, "OP_SET_UPVALUE") {}
    };

    class Equal : public _Instruction {
    public:
        Equal() : _Instruction(OpCode::Equal, 1, "OP_EQUAL") {}
    };

    class Inherit : public _Instruction {
    public:
        Inherit() : _Instruction(OpCode::Inherit, 1, "OP_INHERIT") {}
    };

    class Nil : public _Instruction {
    public:
        Nil() : _Instruction(OpCode::Nil, 1, "OP_NIL") {}
    };
    class Not : public _Instruction {
    public:
        Not() : _Instruction(OpCode::Not, 1, "OP_NOT") {}
    };

    class JumpIfFalse : public _OpAndValueInstruction<uint16_t> {
    public:
        JumpIfFalse(const std::byte* buffer) : _OpAndValueInstruction<uint16_t>(buffer, OpCode::JumpIfFalse, "OP_JUMP_IF_FALSE") {}
    };
    class Jump : public _OpAndValueInstruction<uint16_t> {
    public:
        Jump(const std::byte* buffer) : _OpAndValueInstruction<uint16_t>(buffer, OpCode::Jump, "OP_JUMP") {}
    };

    class Loop : public _OpAndValueInstruction<uint16_t> {
    public:
        Loop(const std::byte* buffer) : _OpAndValueInstruction<uint16_t>(buffer, OpCode::Loop, "OP_LOOP") {}
    };

    class True : public _Instruction {
    public:
        True() : _Instruction(OpCode::True, 1, "OP_TRUE") {}
    };
    class False : public _Instruction {
    public:
        False() : _Instruction(OpCode::True, 1, "OP_FALSE") {}
    };

    class Negate : public _Instruction {
    public:
        Negate() : _Instruction(OpCode::Negate, 1, "OP_NEGATE") {}
    };
    class Print : public _Instruction {
    public:
        Print() : _Instruction(OpCode::Print, 1, "OP_PRINT") {}
    };

    class Pop : public _Instruction {
    public:
        Pop() : _Instruction(OpCode::Pop, 1, "OP_POP") {}
    };
    class CloseUpValue : public _Instruction {
    public:
        CloseUpValue() : _Instruction(OpCode::CloseUpValue, 1, "OP_CLOSE_UPVALUE") {}
    };

    class Return : public _Instruction {
    public:
        Return() : _Instruction(OpCode::Return, 1, "OP_RETURN") {}
    };

    class Unknown : public _Instruction {
    public:
        Unknown(const std::byte* buffer) : _Instruction(OpCode{static_cast<uint8_t>(*buffer)}, 1, "OP_UNKNOWN: " + std::to_string(static_cast<uint32_t>(*buffer))) {}
    };
    inline std::string toBinaryName(std::byte opcodeByte) {
        OpCode opcode{static_cast<uint8_t>(opcodeByte)};
        switch (opcode) {
        case OpCode::Add:
            return "OP_ADD";
        case OpCode::Subtract:
            return "OP_SUBTRACT";
        case OpCode::Multiply:
            return "OP_MULTIPLY";
        case OpCode::Divide:
            return "OP_DIVIDE";
        case OpCode::Less:
            return "OP_LESS";
        case OpCode::Greater:
            return "OP_GREATER";
        case OpCode::BitwiseAnd:
            return "OP_BITWISE_AND";
        case OpCode::BitwiseOr:
            return "OP_BITWISE_OR";
        default:
            return "Unknown Binary Op";
        }
    }

    template <typename T>
    inline std::function<T(T, T)> toBinaryOp(std::byte opcodeByte) {
        OpCode opcode{static_cast<uint8_t>(opcodeByte)};
        switch (opcode) {
        case OpCode::Add:
            return std::plus<T>();
        case OpCode::Subtract:
            return std::minus<T>();
        case OpCode::Multiply:
            return std::multiplies<T>();
        case OpCode::Divide:
            return std::divides<T>();
        case OpCode::BitwiseAnd:
            return [](T v1, T v2) {
                return T(uint64_t(round(v1)) & uint64_t(round(v2)));
            };
        case OpCode::BitwiseOr:
            return [](T v1, T v2) {
                return T(uint64_t(round(v1)) | uint64_t(round(v2)));
            };
        default:
            throw lox::Exception("Unknown binary operation", nullptr);
        }
    }
    template <typename T>
    inline std::function<bool(T, T)> toBinaryPredicate(std::byte opcodeByte) {
        OpCode opcode{static_cast<uint8_t>(opcodeByte)};
        switch (opcode) {
        case OpCode::Less:
            return std::less<T>();
        case OpCode::Greater:
            return std::greater<T>();
        default:
            throw lox::Exception("Unknown binary predicate", nullptr);
        }
    }
    class Binary : public _Instruction {
    public:
        using ArithmeticOp = std::function<double(double, double)>;
        Binary(const std::byte* buffer) : _Instruction(OpCode{static_cast<uint8_t>(*buffer)}, 1, toBinaryName(*buffer)), opcodeByte(*buffer) {}
        ArithmeticOp getOp() const { return toBinaryOp<double>(opcodeByte); }

    private:
        std::byte opcodeByte;
    };

    class BinaryPredicate : public _Instruction {
    public:
        BinaryPredicate(const std::byte* buffer) : _Instruction(OpCode{static_cast<uint8_t>(*buffer)}, 1, toBinaryName(*buffer)), opcodeByte(*buffer) {}
        using ArithmeticPredicate = std::function<bool(double, double)>;
        ArithmeticPredicate getPredicate() const { return toBinaryPredicate<double>(opcodeByte); }

    private:
        std::byte opcodeByte;
    };

    class Instruction {
    public:
        Instruction();
        // buffer should be at the current instruction, and offset says how far this point is from the original buffer
        Instruction(const std::byte* buffer, size_t offset, const Chunk* chunk);
        Instruction(const Instruction& rhs) = default;
        Instruction& operator=(const Instruction& rhs) = default;
        Instruction(Instruction&& rhs) = default;
        Instruction& operator=(Instruction&& rhs) = default;

        using InstVariant = std::variant<Binary, BinaryPredicate, Call, ClassOp, ClosureOp, Constant, DefineGlobal, GetGlobal, Equal, False, LongConstant,
                                         LongDefineGlobal, LongGetGlobal, Negate, Nil, Not, Print, Pop, Return, MethodOp, SetGlobal, LongSetGlobal, GetLocal,
                                         SetLocal, GetUpValue, SetUpValue, JumpIfFalse, Jump, Loop, True, CloseUpValue, GetProperty, SetProperty, Unknown, Invoke,
                                         Initializer, Inherit, InnerInvoke>;
        InstVariant instruction() const;
        size_t offset() const;
        size_t size() const;

    private:
        InstVariant _instruction;
        size_t _offset = 0;
        const Chunk* chunk = nullptr;
    };

    class Chunk {
    public:
        class InstructionIterator {
        public:
            InstructionIterator(Vector<std::byte>::const_iterator it, const Chunk* chunk);
            // Dereference operator
            Instruction& operator*();

            // Increment operator (prefix)
            InstructionIterator& operator++();

            // Increment operator (postfix)
            InstructionIterator operator++(int);

            InstructionIterator& operator+=(size_t index);

            void resetBy(int index);

            // Inequality operator
            bool operator!=(const InstructionIterator& other) const;

            Instruction* operator->();

            friend ptrdiff_t operator-(const InstructionIterator& lhs, const InstructionIterator& rhs) {
                return lhs.current - rhs.current;
            }

        private:
            void parseInstruction();
            Vector<std::byte>::const_iterator current;
            Instruction instruction;
            size_t offset = 0;
            bool parsed = false;
            const Chunk* chunk = nullptr;
        };

        using iterator_type = InstructionIterator;
        void write(lox::OpCode value, size_t line);
        void write(uint8_t value, size_t line);
        void write(std::byte value, size_t line);
        void writeAt(size_t index, std::byte value);
        void write(size_t value, size_t line);
        void writeConstant(Value value, size_t line);
        void writeOpAndIndex(OpCode small, OpCode large, size_t value, size_t line);
        Value getConstant(size_t index) const;

        size_t addConstant(Value value);
        size_t getLineNumber(size_t offset) const;

        InstructionIterator begin() const;

        InstructionIterator end() const;
        size_t size() const;

    private:
        Vector<std::byte> data;
        // line number and count of instructions
        // can't use pair because our allocator doesn't call constructors
        // so we have two sixteen bit fields in our uint32_t
        Vector<uint32_t> lines;
        Vector<Value> values;
    };
}  // namespace lox

#endif