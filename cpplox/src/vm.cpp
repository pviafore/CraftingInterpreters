#include "vm.h"

#include <print>

#include "chunk.h"
#include "debug.h"
#include "optional.h"

template <class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

namespace lox {
    InterpretResult VM::interpret(const Chunk& chunk) {
        // I might have to deal with raw bytes rather than instruction iterator for this
        return run(chunk);
    }

    InterpretResult VM::run(const Chunk& chunk) {
        auto ip = chunk.begin();
        Optional<InterpretResult> returnCode;
        while (ip != chunk.end()) {
            if (diagnosticMode) {
                std::println("{}", stack);
            }
            auto instruction = *ip++;
            if (diagnosticMode) {
                disassembleInstruction(chunk, instruction);
            }
            std::visit(
                overload{
                    [this](const Binary& bin) { const Value b = stack.pop(); const Value a = stack.pop(); stack.push(bin.getOp()(a, b)); },
                    [&chunk, this](const Constant& c) { stack.push(chunk.getConstant(c.value())); },
                    [&chunk, this](const LongConstant& c) { stack.push(chunk.getConstant(c.value())); },
                    [this](const Negate&) { stack.push(-stack.pop()); },
                    [&returnCode, this](const Return&) { std::println("{}", stack.pop()); returnCode = InterpretResult::Ok; },
                    [&returnCode](const Unknown&) { returnCode = InterpretResult::CompileError; }},
                instruction.instruction());
            if (returnCode.hasValue()) {
                return returnCode.value();
            }
        }
        return InterpretResult::Ok;
    }
}