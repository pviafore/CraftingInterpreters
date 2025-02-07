#ifndef CPPLOX_DEBUG_H_
#define CPPLOX_DEBUG_H_
#include <format>
#include <sstream>
#include <string_view>

#include "chunk.h"

namespace lox {
    void printChunk(const lox::Chunk& chunk, std::string_view name);
    void disassembleInstruction(const lox::Chunk& chunk, const lox::Instruction& instruction);
    void formatInstruction(std::ostringstream& out, const lox::Chunk& chunk, const lox::Instruction& instruction);
}
template <>
struct std::formatter<lox::Chunk, char> {
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FmtContext>
    constexpr FmtContext::iterator format(const lox::Chunk& chunk, FmtContext& ctx) const {
        std::ostringstream out;

        for (const auto& instruction : chunk) {
            lox::formatInstruction(out, chunk, instruction);
        }

        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

#endif