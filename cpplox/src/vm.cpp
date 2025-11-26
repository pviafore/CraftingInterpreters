#include "vm.h"

#include <chrono>
#include <print>

#include "chunk.h"
#include "compiler.h"
#include "debug.h"
#include "error.h"
#include "optional.h"
namespace lox {
    const size_t FRAMES_MAX = 64;

    Value clockNative(int, Span<Value>) {
        return double(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    VM::VM() {
        defineNative("clock", clockNative);
    }
    InterpretResult VM::interpret(const String& s) {
        Compiler compiler(s);
        compiler.debugMode = true;
        auto function = compiler.compile();
        if (!function) {
            return InterpretResult::CompileError;
        }
        stack.push(function);
        call(function, 0);
        return run();
    }

    bool areEqual(Value val1, Value val2) {
        if (std::holds_alternative<InternedString>(val1) && std::holds_alternative<InternedString>(val2)) {
            auto s1 = std::get<InternedString>(val1);
            auto s2 = std::get<InternedString>(val2);
            return s1.getHash() == s2.getHash() && s1.begin() == s2.begin() && s1.size() == s2.size();
        }
        return val1 == val2;
    }

    InterpretResult VM::run() {
        Optional<InterpretResult> returnCode;
        try {
            while (!frames.empty() && frames.top().getIp() != frames.top().getFunction()->getChunk()->end()) {
                const auto& chunk = **frames.top().getFunction()->getChunk();
                auto& ip = frames.top().getIp();
                if (diagnosticMode) {
                    std::println("{}", stack);
                }
                auto instruction = *ip;
                if (diagnosticMode) {
                    disassembleInstruction(chunk, instruction);
                }
                bool jumped = false;
                std::visit(
                    overload{
                        [this](const Binary& bin) { binaryOp(bin); },
                        [this](const BinaryPredicate& bin) {
                            const auto b = popNumber();
                            stack.push(bin.getPredicate()(popNumber(), b));
                        },
                        [this, &returnCode](const Call& c) {
                            callValue(stack.peek(c.value()), c.value());
                        },
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
                        [&ip, &jumped, this](const JumpIfFalse& j) { if (isFalsey(stack.peek())) { ip += j.value(); jumped = true;} },
                        [&ip, &jumped, this](const Jump& j) { ip += j.value(); jumped = true; },
                        [&ip, &jumped, this](const Loop& l) { ip.resetBy(l.value()); jumped = true; },
                        [this](const Negate&) { this->negate(); },
                        [this](const Print&) { std::println("{}", stack.pop()); },
                        [this](const Pop&) { stack.pop(); },
                        [this](const Nil&) { stack.push(nullptr); },
                        [this](const Not&) { stack.push(isFalsey(stack.pop())); },
                        [&returnCode, this, &jumped](const Return&) {
                            auto result = stack.pop();
                            frames.pop();
                            if (frames.empty()) {
                                stack.pop();
                            } else {
                                stack.push(result);
                            }
                            jumped = true;
                        },
                        [&chunk, &returnCode, this](const SetGlobal& g) { returnCode = assignGlobal(chunk, g.value()); },
                        [&chunk, &returnCode, this](const LongSetGlobal& g) { returnCode = assignGlobal(chunk, g.value()); },
                        [this](const True&) { stack.push(true); },
                        [&returnCode](const Unknown&) { returnCode = InterpretResult::CompileError; }},
                    instruction.instruction());

                if (!jumped) {
                    ip++;
                }
                if (returnCode.hasValue() && returnCode.value() != InterpretResult::Ok) {
                    return returnCode.value();
                }
            }
        } catch (lox::Exception& e) {
            std::println(std::cerr, "Error: {}", e.what());
            for (auto f : views::reversed<DynamicStack<CallFrame>>(frames)) {
                std::println(std::cerr, "[Line {} in {}]", f.getFunction()->getChunk()->getLineNumber(f.getIp()->offset()), f.getFunction()->getName());
            }
            while (!frames.empty()) {
                frames.pop();
            }
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
        stack.push(frames.top().peek(number));
    }

    void VM::assignLocal(size_t number) {
        frames.top().assign(number, stack.peek());
    }

    void VM::callValue(Value callee, int argCount) {
        std::visit(
            overload{
                [this, argCount](SharedPtr<Function> func) { call(func, argCount); },
                [this, argCount](SharedPtr<NativeFunction> func) {
                    auto result = func->invoke(argCount, Span(stack.begin() + stack.size() - argCount, argCount));
                    for (auto i = 0; i < argCount; i++) {
                        stack.pop();  // pop args
                    }
                    // pop func
                    stack.pop();
                    stack.push(result);
                },
                [this](auto) { throw Exception("Can only call functions and classes", nullptr); }},
            callee);
    }

    void VM::call(SharedPtr<Function> func, int argCount) {
        if (argCount != func->getArity()) {
            throw Exception(std::format("Expected {} arguments but got {}.", func->getArity(), argCount).c_str(), nullptr);
        }
        if (frames.size() == FRAMES_MAX) {
            throw Exception("Stack overflow.", nullptr);
        }
        frames.push(CallFrame{func, stack, argCount});
    }

    void VM::defineNative(StringView name, NativeFunction::Function f) {
        globals.insert(name, SharedPtr<NativeFunction>::Make(f));
    }
}