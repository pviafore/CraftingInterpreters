#include "debug.h"

#include <iomanip>
#include <print>

template <class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;
namespace lox {

    void printChunk(const lox::Chunk& chunk, std::string_view name) {
        std::println("== {} ==", name);
        std::println("{}", chunk);
    }

    void constantInstruction(std::ostringstream& out, const Chunk& chunk, const lox::Constant& c) {
        out << std::left << std::setw(16) << "OP_CONSTANT" << " " << std::setw(4) << c.value() << " " << chunk.getConstant(c.value());
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
            [&out](Return&) { out << "OP_RETURN"; },
            [&out, &chunk, &instruction](Constant& c) { constantInstruction(out, chunk, c); },
            [&out](Unknown& u) { out << "Unknown Op Code" << std::to_underlying(u.opcode); },
        };
        std::visit(overloads, inst);
        out << "\n";
    }

}