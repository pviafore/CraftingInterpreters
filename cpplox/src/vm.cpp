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

    Expected<Value, String> clockNative(Span<Value>) {
        return Value{double(std::chrono::steady_clock::now().time_since_epoch().count())};
    }

    Expected<Value, String> hasfieldNative(Span<Value> values) {
        if (values.size() != 2) {
            return String("Must have three arguments");
        }
        auto val1 = *values.begin();
        auto val2 = *(values.begin() + 1);
        if (!std::holds_alternative<SharedPtr<Instance>>(val1) && !std::holds_alternative<InternedString>(val2)) {
            return String{"Must pass in an instance and field name"};
        }
        auto instance = std::get<SharedPtr<Instance>>(val1);
        auto fieldName = std::get<InternedString>(val2);
        return Value{instance->hasField(fieldName)};
    }

    Expected<Value, String> setfieldNative(Span<Value> values) {
        if (values.size() != 3) {
            return String("Must have three arguments");
        }
        auto val1 = *values.begin();
        auto val2 = *(values.begin() + 1);
        auto val3 = *(values.begin() + 2);
        if (!std::holds_alternative<SharedPtr<Instance>>(val1) && !std::holds_alternative<InternedString>(val2)) {
            return String{"Must pass in an instance and field name"};
        }
        auto instance = std::get<SharedPtr<Instance>>(val1);
        auto fieldName = std::get<InternedString>(val2);
        instance->setField(fieldName, val3);
        return Value{nullptr};
    }

    Expected<Value, String> deletefieldNative(Span<Value> values) {
        if (values.size() != 2) {
            return String("Must have three arguments");
        }
        auto val1 = *values.begin();
        auto val2 = *(values.begin() + 1);
        if (!std::holds_alternative<SharedPtr<Instance>>(val1) && !std::holds_alternative<InternedString>(val2)) {
            return String{"Must pass in an instance and field name"};
        }
        auto instance = std::get<SharedPtr<Instance>>(val1);
        auto fieldName = std::get<InternedString>(val2);
        instance->deleteField(fieldName);
        return Value{nullptr};
    }

    Expected<Value, String> random(Span<Value> values) {
        if (values.size() != 2) {
            return String{"Must have two arguments"};
        }
        auto val1 = *values.begin();
        auto val2 = *(values.begin() + 1);
        if (!std::holds_alternative<double>(val1) || !std::holds_alternative<double>(val2)) {
            return String{"Must pass in numbers to random"};
        }
        int num1 = int(std::get<double>(val1));
        int num2 = int(std::get<double>(val2));
        if (num1 >= num2) {
            return String{"Second number must be bigger than first number"};
        }
        return Value{double(num1 + rand() % (num2 - num1))};
    }

    VM::VM() {
        defineNative("clock", clockNative, 0);
        defineNative("random", random, 2);
        defineNative("hasfield", hasfieldNative, 2);
        defineNative("deletefield", deletefieldNative, 2);
        defineNative("setfield", setfieldNative, 3);
    }
    InterpretResult VM::interpret(const String& s) {
        Compiler compiler(s);
        auto function = compiler.compile();
        if (!function) {
            return InterpretResult::CompileError;
        }
        auto closure = SharedPtr<Closure>::Make(function);
        stack.push(closure);
        call(Callable{closure}, 0);
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
                        [&chunk, this](const ClosureOp& c) {
                            auto value = chunk.getConstant(c.value());
                            if (!std::holds_alternative<SharedPtr<Function>>(value)) {
                                throw Exception("Closure was not a function", nullptr);
                            }
                            auto func = std::get<SharedPtr<Function>>(value);
                            auto closure = SharedPtr<Closure>::Make(func);
                            stack.push(closure);
                            for (auto upvalue : c.getUpValues()) {
                                if (upvalue.isLocal) {
                                    closure->addUpValue(captureUpValue(stack.begin() + frames.top().getOffset() + upvalue.index));
                                } else {
                                    auto sp = std::get<SharedPtr<Closure>>(frames.top().getCallable())->getUpValue(upvalue.index);
                                    closure->addUpValue(sp);
                                }
                            }
                        },
                        [&chunk, this](const Constant& c) {
                            stack.push(chunk.getConstant(c.value()));
                        },
                        [&chunk, this](const ClassOp& c) { stack.push(SharedPtr<Class>::Make(std::get<InternedString>(chunk.getConstant(c.value())))); },
                        [&chunk, this](const LongConstant& c) {
                            stack.push(chunk.getConstant(c.value()));
                        },
                        [&chunk, this](const DefineGlobal& d) { defineGlobal(chunk, d.value()); },
                        [&chunk, this](const LongDefineGlobal& d) { defineGlobal(chunk, d.value()); },
                        [this](const Equal&) { stack.push(areEqual(stack.pop(), stack.pop())); },
                        [this](const False&) { stack.push(false); },
                        [&chunk, &returnCode, this](const GetGlobal& g) { returnCode = pushGlobal(chunk, g.value()); },
                        [&chunk, &returnCode, this](const LongGetGlobal& g) { returnCode = pushGlobal(chunk, g.value()); },
                        [&chunk, &returnCode, this](const GetLocal& g) { pushLocal(g.value()); },
                        [&chunk, &returnCode, this](const SetLocal& s) { assignLocal(s.value()); },
                        [&chunk, &returnCode, this](const GetProperty& g) {
                            if (!std::holds_alternative<SharedPtr<Instance>>(stack.peek())) {
                                throw Exception("Only instances have properties.", nullptr);
                            }
                            auto instance = std::get<SharedPtr<Instance>>(stack.peek());
                            auto name = std::get<InternedString>(chunk.getConstant(g.value()));
                            auto value = instance->getField(name);
                            if (value.hasValue()) {
                                stack.pop();
                                stack.push(value.value());
                            } else {
                                bindMethod(instance->getClass(), name);
                            }
                        },
                        [&chunk, &returnCode, this](const SetProperty& s) {
                            if (!std::holds_alternative<SharedPtr<Instance>>(stack.peek(1))) {
                                throw Exception("Only instances have properties.", nullptr);
                            }

                            auto instance = std::get<SharedPtr<Instance>>(stack.peek(1));
                            auto name = std::get<InternedString>(chunk.getConstant(s.value()));
                            instance->setField(name, stack.peek());
                            auto v = stack.pop();
                            stack.pop();
                            stack.push(v);
                        },
                        [&chunk, &returnCode, this](const GetUpValue& g) {
                            if (!std::holds_alternative<SharedPtr<Closure>>(frames.top().getCallable())) {
                                throw Exception("Not a closure", nullptr);
                            }
                            Value* value = (std::get<SharedPtr<Closure>>(frames.top().getCallable())->getUpValue(g.value())->location);
                            assert(value);
                            stack.push(*value);
                        },
                        [&chunk, &returnCode, this](const SetUpValue& s) { std::get<SharedPtr<Closure>>(frames.top().getCallable())->setUpValue(s.value(), stack.peek()); },
                        [&ip, &jumped, this](const JumpIfFalse& j) {
                            if (isFalsey(stack.peek())) {
                                ip += j.value();
                                jumped = true;
                            } },
                        [&ip, &jumped, this](const Jump& j) {
                            ip += j.value();
                            jumped = true; },
                        [&ip, &jumped, this](const Loop& l) {
                            ip.resetBy(l.value());
                            jumped = true; },
                        [this](const Inherit&) {
                            auto superclass = stack.peek(1);
                            if (!std::holds_alternative<SharedPtr<Class>>(superclass)) {
                                throw Exception("Superclass must be a class.", nullptr);
                            }
                            auto subclass = std::get<SharedPtr<Class>>(stack.peek());
                            subclass->inherit(*std::get<SharedPtr<Class>>(superclass));
                            stack.pop();
                        },
                        [&chunk, this](const MethodOp& m) {
                            defineMethod(std::get<InternedString>(chunk.getConstant(m.value())));
                        },
                        [&chunk, this](const Initializer& i) { defineMethod(std::get<InternedString>(chunk.getConstant(i.value())), true); },
                        [this](const Negate&) {
                            this->negate();
                        },
                        [this](const Print&) { std::println("{}", stack.pop()); },
                        [this](const Pop&) { stack.pop(); },
                        [this](const CloseUpValue&) { closeUpValues(stack.begin() + stack.size() - 1); stack.pop(); },
                        [this](const Nil&) { stack.push(nullptr); },
                        [this](const Not&) { stack.push(isFalsey(stack.pop())); },
                        [&returnCode, this, &jumped](const Return&) {
                            auto result = stack.pop();
                            closeUpValues(stack.begin() + frames.top().getOffset());
                            auto lastFrame = frames.pop();
                            if (frames.empty()) {
                                stack.pop();
                            } else {
                                while (stack.size() > lastFrame.getOffset()) {
                                    stack.pop();  // go back down to before the offset
                                }
                                stack.push(result);
                            }
                            jumped = true;
                        },
                        [&chunk, &returnCode, this](const SetGlobal& g) { returnCode = assignGlobal(chunk, g.value()); },
                        [&chunk, &returnCode, this](const LongSetGlobal& g) { returnCode = assignGlobal(chunk, g.value()); },
                        [this](const True&) { stack.push(true); },
                        [this, &chunk](const Invoke& i) {
                            auto name = chunk.getConstant(i.value());
                            invoke(std::get<InternedString>(name), i.getArgumentCount());
                        },
                        [this, &chunk](const InnerInvoke& i) {
                            auto methodName = std::get<InternedString>(chunk.getConstant(i.value()));
                            auto superclassName = std::get<InternedString>(chunk.getConstant(i.getMethodName()));
                            auto instance = std::get<SharedPtr<Instance>>(stack.peek(i.getArgumentCount()));
                            innerInvoke(instance->getClass(), superclassName, methodName, i.getArgumentCount());
                        },
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
            while (!stack.empty()) {
                stack.pop();
            }
            return InterpretResult::RuntimeError;
        }
        return InterpretResult::Ok;
    }

    SharedPtr<UpValueObj> VM::captureUpValue(DynamicStack<Value>::iterator iter) {
        auto node = openUpValues.findNextClosest(SharedPtr<UpValueObj>::Make(iter));
        if (node && node->value->location == iter) {
            return node->value;
        }

        auto created = SharedPtr<UpValueObj>::Make(iter);
        openUpValues.insertBefore(created, node);
        return created;
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
                [this, argCount](SharedPtr<Closure> func) { call(func, argCount); },
                [this, argCount](SharedPtr<Function> func) { call(func, argCount); },
                [this, argCount](SharedPtr<Class> cls) {
                    stack[stack.size() - argCount - 1] = SharedPtr<Instance>::Make(cls);
                    auto init = cls->getInitializer();
                    if (init.hasValue()) {
                        call(toCallable(init.value()), argCount);
                    }
                },
                [this, argCount](SharedPtr<BoundMethod> method) { stack[stack.size() - argCount - 1] = method->getReceiver(); call(method->getMethod(), argCount); },
                [this, argCount](SharedPtr<NativeFunction> func) {
                    auto result = func->invoke(argCount, Span(stack.begin() + stack.size() - argCount, argCount));
                    for (auto i = 0; i < argCount; i++) {
                        stack.pop();  // pop args
                    }
                    // pop func
                    stack.pop();
                    if (!result.hasValue()) {
                        throw Exception(result.error().c_str(), nullptr);
                    }
                    stack.push(result.value());
                },
                [this](auto) { throw Exception("Can only call functions and classes", nullptr); }},
            callee);
    }

    void VM::call(Callable func, size_t argCount) {
        if (argCount != lox::getFunction(func)->getArity()) {
            throw Exception(std::format("Expected {} arguments but got {}.", lox::getFunction(func)->getArity(), argCount).c_str(), nullptr);
        }
        if (frames.size() == FRAMES_MAX) {
            throw Exception("Stack overflow.", nullptr);
        }
        if (stack.size() < argCount + 1) {
            throw Exception("Stack corruption", nullptr);
        }
        frames.push(CallFrame{func, stack, stack.size() - argCount - 1});
    }

    void VM::defineNative(StringView name, NativeFunction::Func f, size_t args) {
        globals.insert(name, SharedPtr<NativeFunction>::Make(f, args));
    }

    void VM::closeUpValues(const DynamicStack<Value>::iterator iter) {
        while (openUpValues.front() && openUpValues.front()->value->location >= iter) {
            auto upvalue = openUpValues.front()->value;
            upvalue->close();
            openUpValues.popFront();
        }
    }

    void VM::defineMethod(InternedString name, bool isInitializer) {
        auto method = stack.peek();
        auto cls = std::get<SharedPtr<Class>>(stack.peek(1));
        if (isInitializer) {
            cls->setInitializer(method);
        } else {
            cls->setMethod(name, method);
        }
        stack.pop();
    }

    void VM::bindMethod(SharedPtr<Class> cls, InternedString name) {
        auto value = cls->getMethod(name);
        if (!value) {
            auto s = std::format("Undefined property {}", name.string());
            throw Exception(s.c_str(), nullptr);
        }

        SharedPtr<BoundMethod> method = std::visit(
            overload{
                [this](SharedPtr<Function>& f) { return SharedPtr<BoundMethod>::Make(stack.peek(), Callable{f}); },
                [this](SharedPtr<Closure>& f) { return SharedPtr<BoundMethod>::Make(stack.peek(), Callable{f}); },
                [](auto) -> SharedPtr<BoundMethod> { throw Exception("Not callable", nullptr); return nullptr; }},
            value.value());
        stack.pop();
        stack.push(method);
    }

    void VM::invoke(InternedString name, uint8_t argCount) {
        auto value = stack.peek(argCount);
        if (!std::holds_alternative<SharedPtr<Instance>>(value)) {
            throw Exception("Only instances have methods.", nullptr);
        }
        auto receiver = std::get<SharedPtr<Instance>>(value);
        auto v = receiver->getField(name);
        if (v.hasValue()) {
            stack[stack.size() - argCount - 1] = v.value();
            return callValue(v.value(), argCount);
        }
        return invokeFromClass(receiver->getClass(), name, argCount);
    }

    void VM::invokeFromClass(SharedPtr<Class> cls, InternedString name, uint8_t argCount) {
        auto method = cls->getMethod(name);
        if (!method.hasValue()) {
            auto s = std::format("Undefined property {}", name.string());
            throw Exception(s.c_str(), nullptr);
        }
        return call(toCallable(method.value()), argCount);
    }

    void VM::innerInvoke(SharedPtr<Class> cls, InternedString superclassName, InternedString name, uint8_t argCount) {
        auto method = cls->getMethod(name, superclassName);
        if (!method.hasValue()) {
            auto s = std::format("Undefined property {}", name.string());
            throw Exception(s.c_str(), nullptr);
        }
        return call(toCallable(method.value()), argCount);
    }
}