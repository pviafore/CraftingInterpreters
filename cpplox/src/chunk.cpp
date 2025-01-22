#include "chunk.h"

#include <limits>

#include "loxexception.h"
namespace lox {

    std::byte unknown = static_cast<std::byte>(std::to_underlying(OpCode::Unknown));
    Instruction::Instruction() : _instruction(Unknown{&unknown}), _offset(0) {}

    Instruction::InstVariant makeInstruction(const std::byte* buffer) {
        OpCode code = OpCode{static_cast<uint8_t>(*(buffer))};
        switch (code) {
        case OpCode::Return:
            return Return{};
        case OpCode::Constant:
            return Constant(buffer);
        default:
            return Unknown{buffer};
        }
    }
    Instruction::Instruction(const std::byte* buffer, size_t offset) : _instruction(makeInstruction(buffer)), _offset(offset) {
    }

    Instruction::InstVariant Instruction::instruction() const {
        return _instruction;
    }
    size_t Instruction::offset() const {
        return _offset;
    }
    size_t Instruction::size() const {
        return std::visit([](const auto& i) { return i.size; }, _instruction);
    }

    uint8_t Constant::value() const {
        return constantAddress;
    }

    Chunk::InstructionIterator::InstructionIterator(Vector<std::byte>::const_iterator it) {
        this->current = it;
        parseInstruction();
    }

    // Dereference operator
    Instruction& Chunk::InstructionIterator::operator*() {
        parseInstruction();
        return instruction;
    }

    // Increment operator (prefix)
    Chunk::InstructionIterator& Chunk::InstructionIterator::operator++() {
        parseInstruction();
        current += instruction.size();
        offset += instruction.size();
        parsed = false;
        return *this;
    }

    // Inequality operator
    bool Chunk::InstructionIterator::operator!=(const InstructionIterator& other) const {
        return current != other.current;
    }

    void Chunk::InstructionIterator::parseInstruction() {
        if (parsed) {
            return;
        }
        instruction = Instruction(current, offset);
        parsed = true;
    }
    void Chunk::write(lox::OpCode value, size_t line) {
        data.push_back(std::byte{std::to_underlying(value)});
        lines.push_back(line);
    }

    void Chunk::write(uint8_t value, size_t line) {
        data.push_back(std::byte{value});
        lines.push_back(line);
    }

    size_t Chunk::getLineNumber(size_t offset) const {
        return lines[offset];
    }

    Chunk::InstructionIterator Chunk::begin() const {
        return InstructionIterator(data.begin());
    }

    Chunk::InstructionIterator Chunk::end() const {
        return InstructionIterator(data.end());
    }

    uint8_t Chunk::addConstant(Value value) {
        if (values.size() == std::numeric_limits<uint8_t>::max()) {
            throw lox::Exception("Can only have 255 constants", nullptr);
        }
        values.push_back(value);
        return values.size() - 1;
    }

    Value Chunk::getConstant(uint8_t index) const {
        return values[index];
    }
}