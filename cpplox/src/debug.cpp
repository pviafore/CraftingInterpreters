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
    void withValue(std::ostringstream& out, const Chunk& chunk, const I& i) {
        out << std::format("{:<32}{}({})", i.name, chunk.getConstant(i.value()), i.value());
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
            [&out, &chunk, &instruction](Constant& o) { withValue(out, chunk, o); },
            [&out, &chunk, &instruction](LongConstant& o) { withValue(out, chunk, o); },
            [&out, &chunk, &instruction](DefineGlobal& o) { withValue(out, chunk, o); },
            [&out, &chunk, &instruction](LongDefineGlobal& o) { withValue(out, chunk, o); },
            [&out, &chunk, &instruction](GetGlobal& o) { withValue(out, chunk, o); },
            [&out, &chunk, &instruction](LongGetGlobal& o) { withValue(out, chunk, o); },
            [&out, &chunk, &instruction](SetGlobal& o) { withValue(out, chunk, o); },
            [&out, &chunk, &instruction](LongSetGlobal& o) { withValue(out, chunk, o); },
            // simple instructions that are just a name
            [&out](auto i) { out << i.name; },
        };
        std::visit(overloads, inst);
        out << "\n";
    }
}