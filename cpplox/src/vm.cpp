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

    bool areEqual(Value val1, Value val2) {
        if (isString(val1) && isString(val2)) {
            auto s = std::holds_alternative<StringView>(val1) ? std::get<StringView>(val1)
                                                              : StringView(**std::get<SharedPtr<String>>(val1));
            auto s2 = std::holds_alternative<StringView>(val2) ? std::get<StringView>(val2)
                                                               : StringView(**std::get<SharedPtr<String>>(val2));
            return s == s2;
        } else {
            return val1 == val2;
        }
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
                        [this](const Binary& bin) { binaryOp(bin); },
                        [this](const BinaryPredicate& bin) { const auto b = popNumber(); stack.push(bin.getPredicate()(popNumber(), b)); },
                        [&chunk, this](const Constant& c) { stack.push(chunk.getConstant(c.value())); },
                        [&chunk, this](const LongConstant& c) { stack.push(chunk.getConstant(c.value())); },
                        [this](const Equal&) { stack.push(areEqual(stack.pop(), stack.pop())); },
                        [this](const False&) { stack.push(false); },
                        [this](const Negate&) { this->negate(); },
                        [this](const Nil&) { stack.push(nullptr); },
                        [this](const Not&) { stack.push(isFalsey(stack.pop())); },
                        [&returnCode, this](const Return&) { std::println("{}", stack.pop()); returnCode = InterpretResult::Ok; },
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

    void VM::binaryOp(const Binary& bin) {
        if (isNumber(stack.peek(0)) && isNumber(stack.peek(1))) {
            const auto b = popNumber();
            stack.push(bin.getOp()(popNumber(), b));
        } else if (isString(stack.peek(0)) && isString(stack.peek(1)) && bin.opcode == OpCode::Add) {
            auto val2 = stack.pop();
            auto val1 = stack.pop();
            auto s = SharedPtr<String>::Make(std::holds_alternative<StringView>(val1) ? String{std::get<StringView>(val1)} : **std::get<SharedPtr<String>>(val1));
            if (std::holds_alternative<StringView>(val2)) {
                s->push_back(std::get<StringView>(val2));
            } else {
                s->push_back(**std::get<SharedPtr<String>>(val2));
            }
            stack.push(s);
        } else {
            throw lox::Exception("Invalid type for binary expression", nullptr);
        }
    }

    void VM::verifyNumber(size_t stackIndex) const {
        if (!isNumber(stack.peek(stackIndex))) {
            throw lox::Exception("Operand must be a number", nullptr);
        }
    }
    void VM::verifyString(size_t stackIndex) const {
        if (!isString(stack.peek(stackIndex))) {
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