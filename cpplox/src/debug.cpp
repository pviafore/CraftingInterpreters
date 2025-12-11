#include "debug.h"

#include <iomanip>
#include <print>

#include "value.h"
namespace lox {

    void printChunk(const lox::Chunk& chunk, std::string_view name) {
        std::println("== {} ==", name);
        std::println("{}", chunk);
    }

    template <typename I>
    void withConstant(std::ostringstream& out, const Chunk& chunk, const I& i) {
        out << std::format("{:<32}{}({})", i.name, chunk.getConstant(i.value()), i.value());
    }

    void withInvoke(std::ostringstream& out, const Chunk& chunk, const Invoke& i) {
        out << std::format("{:<32}{}({}) ({} args)", i.name, chunk.getConstant(i.value()), i.value(), i.getArgumentCount());
    }

    template <typename I>
    void withRawValue(std::ostringstream& out, const I& i) {
        out << std::format("{:<32}({})", i.name, i.value());
    }

    template <typename I>
    void withJump(std::ostringstream& out, const I& i, int32_t offset) {
        out << std::format("{:<32}({} {})", i.name, i.value(), i.value() + offset);
    }

    void withClosure(std::ostringstream& out, const Chunk& chunk, const ClosureOp& closure) {
        withConstant(out, chunk, closure);
        for (auto upvalue : closure.getUpValues()) {
            out << std::format("\n        |             {} {} ", (upvalue.isLocal ? "local" : "upvalue"), upvalue.index);
        }
    }

    void disassembleInstruction(const lox::Chunk& chunk, const lox::Instruction& instruction) {
        std::ostringstream out;
        formatInstruction(out, chunk, instruction);
        println("{}", out.str());
    }

    void formatInstruction(std::ostringstream& out, const Chunk& chunk, const lox::Instruction& instruction) {
        out << std::setw(4) << std::setfill('0') << instruction.offset() << std::setfill(' ') << " ";
        if (instruction.offset() > 0 && chunk.getLineNumber(instruction.offset()) == chunk.getLineNumber(instruction.offset() - 1)) {
            out << "   | ";
        } else {
            out << std::setw(4) << chunk.getLineNumber(instruction.offset()) << " ";
        }
        auto inst = instruction.instruction();
        auto overloads = overload{
            [&out, &chunk, &instruction](Call& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](ClassOp& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](ClosureOp& o) { withClosure(out, chunk, o); },
            [&out, &chunk, &instruction](Constant& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](LongConstant& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](DefineGlobal& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](LongDefineGlobal& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](GetGlobal& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](LongGetGlobal& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](SetGlobal& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](LongSetGlobal& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](GetLocal& o) { withRawValue(out, o); },
            [&out, &chunk, &instruction](SetLocal& o) { withRawValue(out, o); },
            [&out, &chunk, &instruction](MethodOp& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](Initializer& o) { withConstant(out, chunk, o); },
            [&out, &chunk, &instruction](Invoke& o) { withInvoke(out, chunk, o); },
            [&out, &chunk, &instruction](GetProperty& o) { withRawValue(out, o); },
            [&out, &chunk, &instruction](SetProperty& o) { withRawValue(out, o); },
            [&out, &chunk, &instruction](GetUpValue& o) { withRawValue(out, o); },
            [&out, &chunk, &instruction](SetUpValue& o) { withRawValue(out, o); },
            [&out, &chunk, &instruction](JumpIfFalse& o) { withJump(out, o, instruction.offset()); },
            [&out, &chunk, &instruction](Jump& o) { withJump(out, o, instruction.offset()); },
            [&out, &chunk, &instruction](Loop& o) { withJump(out, o, -1 * instruction.offset()); },
            // simple instructions that are just a name
            [&out](auto i) { out << i.name; },
        };
        std::visit(overloads, inst);
        out << "\n"
            << std::flush;
    }
}