#ifndef CPPLOX_VALUE_H_
#define CPPLOX_VALUE_H_

#include <format>
#include <variant>

#include "memory.h"
#include "object.h"
#include "span.h"
namespace lox {

    // span is used for a "constant" string
    using Value = std::variant<bool, std::nullptr_t, double, SharedPtr<String>, StringView>;

    inline bool isFalsey(Value value) {
        return std::holds_alternative<nullptr_t>(value) || (std::holds_alternative<bool>(value) && !std::get<bool>(value));
    }

    inline bool isNumber(Value value) {
        return std::holds_alternative<double>(value);
    }
    inline bool isString(Value value) {
        return std::holds_alternative<SharedPtr<String>>(value) || std::holds_alternative<StringView>(value);
    }
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
                [&ctx](lox::SharedPtr<lox::String> s) { return "\"" + std::string(s->begin(), s->end()) + "\""; },
                [&ctx](lox::StringView s) { return "\"" + std::string(s.begin(), s.end()) + "\""; },
                [&ctx](bool v) { return v ? "true"s : "false"s; },
                [&ctx](double v) { return std::format("{}", v); },
            },
            v);
        return std::formatter<std::string>::format(s, ctx);
    }
};

#endif