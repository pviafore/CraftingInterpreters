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
        if (std::holds_alternative<InternedString>(val1) && std::holds_alternative<InternedString>(val2)) {
            auto s1 = std::get<InternedString>(val1);
            auto s2 = std::get<InternedString>(val2);
            return s1.getHash() == s2.getHash() && s1.begin() == s2.begin() && s1.size() == s2.size();
        }
        return val1 == val2;
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
                        [this](const BinaryPredicate& bin) {
                    const auto b = popNumber();
                    stack.push(bin.getPredicate()(popNumber(), b)); },
                        [&chunk, this](const Constant& c) { stack.push(chunk.getConstant(c.value())); },
                        [&chunk, this](const LongConstant& c) { stack.push(chunk.getConstant(c.value())); },
                        [&chunk, this](const DefineGlobal& d) { defineGlobal(chunk, d.value()); },
                        [&chunk, this](const LongDefineGlobal& d) { defineGlobal(chunk, d.value()); },
                        [this](const Equal&) { stack.push(areEqual(stack.pop(), stack.pop())); },
                        [this](const False&) { stack.push(false); },
                        [&chunk, &returnCode, this](const GetGlobal& g) { returnCode = pushGlobal(chunk, g.value()); },
                        [&chunk, &returnCode, this](const LongGetGlobal& g) { returnCode = pushGlobal(chunk, g.value()); },
                        [&chunk, &returnCode, this](const GetLocal& g) { pushLocal(g.value()); },
                        [&chunk, &returnCode, this](const SetLocal& s) { assignLocal(s.value()); },
                        [this](const Negate&) { this->negate(); },
                        [this](const Print&) { std::println("{}", stack.pop()); },
                        [this](const Pop&) { stack.pop(); },
                        [this](const Nil&) { stack.push(nullptr); },
                        [this](const Not&) { stack.push(isFalsey(stack.pop())); },
                        [&returnCode, this](const Return&) {},
                        [&chunk, &returnCode, this](const SetGlobal& g) { returnCode = assignGlobal(chunk, g.value()); },
                        [&chunk, &returnCode, this](const LongSetGlobal& g) { returnCode = assignGlobal(chunk, g.value()); },
                        [this](const True&) { stack.push(true); },
                        [&returnCode](const Unknown&) { returnCode = InterpretResult::CompileError; }},
                    instruction.instruction());
                if (returnCode.hasValue() && returnCode.value() != InterpretResult::Ok) {
                    return returnCode.value();
                }
            }
        } catch (lox::Exception& e) {
            std::println(std::cerr, "Error [Line {}] {}", chunk.getLineNumber(ip->offset()), e.what());
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
            stack.push(std::get<InternedString>(val1) + std::get<InternedString>(val2));
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

    void VM::defineGlobal(const Chunk& chunk, uint32_t number) {
        auto constant = chunk.getConstant(number);
        if (!std::holds_alternative<InternedString>(constant)) {
            throw lox::Exception("Did not find a name for the global", nullptr);
        }
        globals.insert(std::get<InternedString>(constant), stack.pop());
    }

    InterpretResult VM::pushGlobal(const Chunk& chunk, uint32_t number) {
        auto constant = chunk.getConstant(number);
        if (!std::holds_alternative<InternedString>(constant)) {
            throw lox::Exception("Not a variable name", nullptr);
        }
        auto s = std::get<InternedString>(constant);
        auto value = globals.get(s);

        if (!value) {
            std::println(std::cerr, "Undefined Variable {}", s.string());
            return InterpretResult::RuntimeError;
        }
        stack.push(*value);
        return InterpretResult::Ok;
    }

    InterpretResult VM::assignGlobal(const Chunk& chunk, uint32_t number) {
        auto constant = chunk.getConstant(number);
        if (!std::holds_alternative<InternedString>(constant)) {
            throw lox::Exception("Not a variable name", nullptr);
        }
        auto s = std::get<InternedString>(constant);
        if (globals.insert(s, stack.peek())) {
            globals.erase(s);
            std::println(std::cerr, "Undefined Variable {}", s.string());
            return InterpretResult::RuntimeError;
        }
        return InterpretResult::Ok;
    }

    void VM::pushLocal(size_t number) {
        stack.push(stack[number]);
    }

    void VM::assignLocal(size_t number) {
        stack[number] = stack.peek();
    }
}