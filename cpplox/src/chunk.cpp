#include "chunk.h"

#include <limits>

#include "loxexception.h"
namespace lox {

    std::byte unknown = static_cast<std::byte>(std::to_underlying(OpCode::Unknown));
    Instruction::Instruction() : _instruction(Unknown{&unknown}), _offset(0) {}

    Instruction::InstVariant makeInstruction(const std::byte* buffer) {
        OpCode code = OpCode{static_cast<uint8_t>(*(buffer))};
        switch (code) {
        case OpCode::Add:
        case OpCode::Divide:
        case OpCode::Multiply:
        case OpCode::Subtract:
            return Binary(buffer);
        case OpCode::Greater:
        case OpCode::Less:
            return BinaryPredicate(buffer);
        case OpCode::Constant:
            return Constant(buffer);
        case OpCode::LongConstant:
            return LongConstant(buffer);
        case OpCode::DefineGlobal:
            return DefineGlobal{buffer};
        case OpCode::LongDefineGlobal:
            return LongDefineGlobal{buffer};
        case OpCode::GetGlobal:
            return GetGlobal{buffer};
        case OpCode::LongGetGlobal:
            return LongGetGlobal{buffer};
        case OpCode::SetGlobal:
            return SetGlobal(buffer);
        case OpCode::LongSetGlobal:
            return LongSetGlobal(buffer);
        case OpCode::SetLocal:
            return SetLocal(buffer);
        case OpCode::GetLocal:
            return GetLocal(buffer);
        case OpCode::Equal:
            return Equal{};
        case OpCode::Negate:
            return Negate{};
        case OpCode::Not:
            return Not{};
        case OpCode::Return:
            return Return{};
        case OpCode::Nil:
            return Nil{};
        case OpCode::Print:
            return Print{};
        case OpCode::Pop:
            return Pop{};
        case OpCode::True:
            return True{};
        case OpCode::False:
            return False{};
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

    uint32_t toAddress(const std::byte* buffer) {
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

    // Increment operator (prefix)
    Chunk::InstructionIterator Chunk::InstructionIterator::operator++(int) {
        InstructionIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    // Inequality operator
    bool Chunk::InstructionIterator::operator!=(const InstructionIterator& other) const {
        return current != other.current;
    }

    Instruction* Chunk::InstructionIterator::operator->() {
        parseInstruction();
        return &instruction;
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
        OpCode opcode = values.size() <= std::numeric_limits<uint8_t>::max() ? OpCode::Constant : OpCode::LongConstant;
        write(opcode, line);
        auto index = addConstant(std::move(value));
        write(index, line);
    }

    void Chunk::writeOpAndIndex(OpCode small, OpCode large, size_t value, size_t line) {
        OpCode opcode = value <= std::numeric_limits<uint8_t>::max() ? small : large;
        write(opcode, line);
        write(value, line);
    }

    size_t Chunk::addConstant(Value value) {
        values.push_back(std::move(value));
        return values.size() - 1;
    }

    void Chunk::write(size_t val, size_t line) {
        if (val > 0xFFFFFF) {
            // 24 bits
            throw lox::Exception("Can only have 24-bit values", nullptr);
        }
        if (val > std::numeric_limits<uint8_t>::max()) {
            write(static_cast<uint8_t>((val >> 16) & 0xFF), line);
            write(static_cast<uint8_t>((val >> 8) & 0xFF), line);
        }
        write(static_cast<uint8_t>(val & 0xFF), line);
    }

    Value Chunk::getConstant(size_t index) const {
        return values[index];
    }
    template <>
    size_t getValueSize<uint8_t>() { return 2; }

    template <>
    size_t getValueSize<uint32_t>() { return 4; }

}