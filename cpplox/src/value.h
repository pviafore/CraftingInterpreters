#ifndef CPPLOX_VALUE_H_
#define CPPLOX_VALUE_H_

#include <format>
#include <variant>

#include "expected.h"
#include "interned.h"
#include "memory.h"
#include "object.h"
#include "span.h"
#include "stack.h"
namespace lox {

    class NativeFunction;
    class Function;
    class Closure;
    struct UpValueObj;
    using Value = std::variant<bool, std::nullptr_t, double, InternedString, SharedPtr<Function>, SharedPtr<NativeFunction>, SharedPtr<Closure>, SharedPtr<UpValueObj>>;

    struct UpValueObj {
        Value* location;
        Value closed = nullptr;

        friend bool operator<(const UpValueObj& obj, const UpValueObj& obj2) {
            // larger locations should be earlier in the list
            return obj.location > obj2.location;
        }

        void close() {
            closed = *location;
            location = &closed;
        }
    };
    class NativeFunction {
    public:
        using Func = std::function<Expected<Value, String>(Span<Value>)>;
        NativeFunction(Func f, size_t argCount);
        Expected<Value, String> invoke(size_t args, Span<Value> values);

        friend bool operator==(const NativeFunction& f1, const NativeFunction& f2) {
            return &f1 == &f2;
        }

    private:
        Func function;
        size_t argCount;
    };

    inline bool isFalsey(Value value) {
        return std::holds_alternative<nullptr_t>(value) || (std::holds_alternative<bool>(value) && !std::get<bool>(value));
    }

    inline bool isString(Value value) {
        return std::holds_alternative<InternedString>(value);
    }

    inline bool isNumber(Value value) {
        return std::holds_alternative<double>(value);
    }
    inline bool isFunction(Value value) {
        return std::holds_alternative<SharedPtr<Function>>(value);
    }

    class Closure {
    public:
        Closure(SharedPtr<Function> f);
        SharedPtr<Function> getFunction() const;
        void addUpValue(SharedPtr<UpValueObj> obj);

        SharedPtr<UpValueObj> getUpValue(size_t index) const;
        void setUpValue(size_t index, Value value);

    private:
        SharedPtr<Function> f;
        Vector<SharedPtr<UpValueObj>> upvalues;
    };
}

template <class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

template <>
struct std::formatter<lox::Value> : std::formatter<std::string> {
    template <class FormatContext>
    FormatContext::iterator format(const lox::Value& v, FormatContext& ctx) const {
        using namespace std::string_literals;
        auto s = std::visit(
            // I technically can use the LoxString here and not cheat and use stl string
            // but I don't care enough for printing to screen, the string formatter is way easier to use
            overload{
                [&ctx](nullptr_t) { return "nil"s; },
                [&ctx](lox::InternedString v) { return "\"" + std::string(v.begin(), v.end()) + "\""; },
                [&ctx](double v) { return std::format("{}", v); },
                [&ctx](bool v) { return v ? "true"s : "false"s; },
                [&ctx](lox::SharedPtr<lox::Function> f) { return std::string(f->getName().str().c_str()); },
                [&ctx](lox::SharedPtr<lox::Closure> f) { return std::string(f->getFunction()->getName().str().c_str()); },
                [&ctx](lox::SharedPtr<lox::NativeFunction>) { return "<native fn>"s; },
                [&ctx](lox::SharedPtr<lox::UpValueObj> v) { return std::format("{}", *(v->location)); }},
            v);
        return std::formatter<std::string>::format(s, ctx);
    }
};

#endif