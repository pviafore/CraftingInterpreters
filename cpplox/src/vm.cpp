#include "vm.h"

#include <print>

#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "error.h"
#include "optional.h"
namespace lox {

    InterpretResult VM::interpret(const String& s) {
        Compiler compiler(s);
        compiler.debugMode = true;
        auto chunk = compiler.compile();
        if (!chunk) {
            return InterpretResult::CompileError;
        }
        return run(chunk.value());
    }

    InterpretResult VM::run(const Chunk& chunk) {
        auto ip = chunk.begin();
        Optional<InterpretResult> returnCode;
        try {
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
                        [this](const Binary& bin) { const auto b = popNumber(); stack.push(bin.getOp()(popNumber(), b)); },
                        [this](const BinaryPredicate& bin) { const auto b = popNumber(); stack.push(bin.getPredicate()(popNumber(), b)); },
                        [&chunk, this](const Constant& c) { stack.push(chunk.getConstant(c.value())); },
                        [&chunk, this](const LongConstant& c) { stack.push(chunk.getConstant(c.value())); },
                        [this](const Equal&) { stack.push(stack.pop() == stack.pop()); },
                        [this](const False&) { stack.push(false); },
                        [this](const Negate&) { this->negate(); },
                        [this](const Nil&) { stack.push(nullptr); },
                        [this](const Not&) { stack.push(isFalsey(stack.pop())); },
                        [&returnCode, this](const Return&) { std::println("{}", std::get<double>(stack.pop())); returnCode = InterpretResult::Ok; },
                        [this](const True&) { stack.push(true); },
                        [&returnCode](const Unknown&) { returnCode = InterpretResult::CompileError; }},
                    instruction.instruction());
                if (returnCode.hasValue()) {
                    return returnCode.value();
                }
            }
        } catch (lox::Exception& e) {
            std::println("Error [Line {}] {}", chunk.getLineNumber(ip->offset()), e.what());
            return InterpretResult::RuntimeError;
        }
        return InterpretResult::Ok;
    }

    void VM::verifyNumber(size_t stackIndex) const {
        if (!std::holds_alternative<double>(stack.peek(stackIndex))) {
            throw lox::Exception("Operand must be a number", nullptr);
        }
    }

    void VM::negate() {
        verifyNumber();
        stack.push(-std::get<double>(stack.pop()));
    }

    double VM::popNumber() {
        verifyNumber();
        return std::get<double>(stack.pop());
    }

}