#include "chunk.h"

#include <limits>

#include "loxexception.h"
namespace lox {

    std::byte unknown = static_cast<std::byte>(std::to_underlying(OpCode::Unknown));
    Instruction::Instruction() : _instruction(Unknown{&unknown}), _offset(0) {}

    Instruction::InstVariant makeInstruction(const std::byte* buffer) {
        OpCode code = OpCode{static_cast<uint8_t>(*(buffer))};
        switch (code) {
        case OpCode::Constant:
            return Constant(buffer);
        case OpCode::LongConstant:
            return LongConstant(buffer);
        case OpCode::Return:
            return Return{};
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

    uint32_t LongConstant::value() const {
        return address;
    }

    uint32_t LongConstant::toAddress(const std::byte* buffer) {
        return (static_cast<uint8_t>(*buffer) << 16) + (static_cast<uint8_t>(*(buffer + 1)) << 8) + static_cast<uint8_t>(*(buffer + 2));
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
        write(std::to_underlying(value), line);
    }

    void Chunk::write(uint8_t value, size_t line) {
        write(std::byte{value}, line);
    }
    void Chunk::write(std::byte value, size_t line) {
        data.push_back(value);
        if (lines.size() > 0) {
            auto lastLine = (lines[lines.size() - 1] & 0xFFFF0000) >> 16;
            if (lastLine == line) {
                lines[lines.size() - 1] = (lines[lines.size() - 1] & 0xFFFF0000) | (((lines[lines.size() - 1] & 0xFFFF) + 1) & 0xFFFF);
                return;
            }
        }
        lines.push_back((line << 16) | 1);
    }

    size_t Chunk::getLineNumber(size_t offset) const {
        size_t currentInstruction = 0;
        for (auto lineAndSize : lines) {
            auto line = (lineAndSize & 0xFFFF0000) >> 16;
            if (offset < currentInstruction + (lineAndSize & 0xFFFF)) {
                return line;
            }
            currentInstruction += lineAndSize & 0xFFFF;
        }
        throw Exception("Instruction not found while getting line number", nullptr);
    }

    Chunk::InstructionIterator Chunk::begin() const {
        return InstructionIterator(data.begin());
    }

    Chunk::InstructionIterator Chunk::end() const {
        return InstructionIterator(data.end());
    }

    void Chunk::writeConstant(Value value, size_t line) {
        if (values.size() == 0xFFFFFF) {
            // 24 bits
            throw lox::Exception("Can only have 2^24 constants", nullptr);
        }
        OpCode opcode = values.size() <= std::numeric_limits<uint8_t>::max() ? OpCode::Constant : OpCode::LongConstant;
        write(opcode, line);
        if (values.size() > std::numeric_limits<uint8_t>::max()) {
            write(static_cast<uint8_t>((values.size() >> 16) & 0xFF), line);
            write(static_cast<uint8_t>((values.size() >> 8) & 0xFF), line);
        }
        write(static_cast<uint8_t>(values.size() & 0xFF), line);
        values.push_back(value);
    }

    Value Chunk::getConstant(size_t index) const {
        return values[index];
    }
}