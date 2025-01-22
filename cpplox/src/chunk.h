#ifndef CPPLOX_CHUNK_H_
#define CPPLOX_CHUNK_H_
#include <utility>
#include <variant>

#include "common.h"
#include "value.h"
#include "vector.h"
namespace lox {
    enum class OpCode : uint8_t {
        Constant,
        Return,
        Unknown
    };

    struct _Instruction {
    public:
        _Instruction(OpCode opcode, size_t size) : opcode(opcode), size(size) {}
        OpCode opcode;
        size_t size = 1;
    };

    class Return : public _Instruction {
    public:
        Return() : _Instruction(OpCode::Return, 1) {}
    };
    class Constant : public _Instruction {
    public:
        Constant(const std::byte* buffer) : _Instruction(OpCode::Constant, 2), constantAddress(static_cast<uint8_t>(*(buffer + 1))) {}
        uint8_t value() const;

    private:
        uint8_t constantAddress;
    };
    class Unknown : public _Instruction {
    public:
        Unknown(const std::byte* buffer) : _Instruction(OpCode{static_cast<uint8_t>(*buffer)}, 1) {}
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

        using InstVariant = std::variant<Return, Constant, Unknown>;
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

            // Inequality operator
            bool operator!=(const InstructionIterator& other) const;

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
        uint8_t addConstant(Value value);
        Value getConstant(uint8_t index) const;

        size_t getLineNumber(size_t offset) const;

        InstructionIterator begin() const;

        InstructionIterator end() const;

    private:
        Vector<std::byte> data;
        Vector<size_t> lines;
        Vector<Value> values;
    };
}  // namespace lox

#endif