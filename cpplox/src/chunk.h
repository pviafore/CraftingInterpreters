#ifndef CPPLOX_CHUNK_H_
#define CPPLOX_CHUNK_H_
#include <functional>
#include <string>
#include <utility>
#include <variant>

#include "common.h"
#include "value.h"
#include "vector.h"
namespace lox {
    enum class OpCode : uint8_t {
        Add,
        Constant,
        DefineGlobal,
        LongDefineGlobal,
        Equal,
        GetGlobal,
        LongGetGlobal,
        Greater,
        Less,
        Nil,
        Not,
        True,
        False,
        Divide,
        LongConstant,
        Multiply,
        Negate,
        Print,
        Pop,
        Return,
        SetGlobal,
        LongSetGlobal,
        Subtract,
        Unknown
    };

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
    class _OpAndValueInstruction : public _Instruction {
    public:
        _OpAndValueInstruction(const std::byte* buffer, OpCode opcode, std::string s) : _Instruction(opcode, getValueSize<T>(), s), constantAddress(*reinterpret_cast<const T*>(buffer + 1)) {}
        T value() const {
            return constantAddress;
        }

    private:
        T constantAddress;
    };

    class Constant : public _OpAndValueInstruction<uint8_t> {
    public:
        Constant(const std::byte* buffer) : _OpAndValueInstruction<uint8_t>(buffer, OpCode::Constant, "OP_CONSTANT") {}
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

    class Equal : public _Instruction {
    public:
        Equal() : _Instruction(OpCode::Equal, 1, "OP_EQUAL") {}
    };

    class Nil : public _Instruction {
    public:
        Nil() : _Instruction(OpCode::Nil, 1, "OP_NIL") {}
    };
    class Not : public _Instruction {
    public:
        Not() : _Instruction(OpCode::Not, 1, "OP_NOT") {}
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
        Instruction(const std::byte* buffer, size_t offset);
        Instruction(const Instruction& rhs) = default;
        Instruction& operator=(const Instruction& rhs) = default;
        Instruction(Instruction&& rhs) = default;
        Instruction& operator=(Instruction&& rhs) = default;

        using InstVariant = std::variant<Binary, BinaryPredicate, Constant, DefineGlobal, GetGlobal, Equal, False, LongConstant, LongDefineGlobal, LongGetGlobal,
                                         Negate, Nil, Not, Print, Pop, Return, SetGlobal, LongSetGlobal, True, Unknown>;
        InstVariant instruction() const;
        size_t offset() const;
        size_t size() const;

    private:
        InstVariant _instruction;
        size_t _offset = 0;
    };

    class Chunk {
    public:
        class InstructionIterator {
        public:
            InstructionIterator(Vector<std::byte>::const_iterator it);
            // Dereference operator
            Instruction& operator*();

            // Increment operator (prefix)
            InstructionIterator& operator++();

            // Increment operator (postfix)
            InstructionIterator operator++(int);

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
        };

        using iterator_type = InstructionIterator;
        void write(lox::OpCode value, size_t line);
        void write(uint8_t value, size_t line);
        void write(std::byte value, size_t line);
        void write(size_t value, size_t line);
        void writeConstant(Value value, size_t line);
        void writeOpAndIndex(OpCode small, OpCode large, size_t value, size_t line);
        Value getConstant(size_t index) const;

        size_t addConstant(Value value);
        size_t getLineNumber(size_t offset) const;

        InstructionIterator begin() const;

        InstructionIterator end() const;

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