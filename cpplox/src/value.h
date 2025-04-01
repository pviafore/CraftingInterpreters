#ifndef CPPLOX_VALUE_H_
#define CPPLOX_VALUE_H_

#include <format>
#include <variant>

#include "memory.h"
#include "object.h"
namespace lox {

    using Value = std::variant<bool, std::nullptr_t, double, SharedPtr<String>>;

    inline bool isFalsey(Value value) {
        return std::holds_alternative<nullptr_t>(value) || (std::holds_alternative<bool>(value) && !std::get<bool>(value));
    }

    inline bool isNumber(Value value) {
        return std::holds_alternative<double>(value);
    }
    inline bool isString(Value value) {
        return std::holds_alternative<SharedPtr<String>>(value);
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
            overload{
                [&ctx](nullptr_t) { return "nil"s; },
                [&ctx](lox::SharedPtr<lox::String> s) { return "\"" + std::string(s->begin(), s->end()) + "\""; },
                [&ctx](bool v) { return v ? "true"s : "false"s; },
                [&ctx](double v) { return std::format("{}", v); },
            },
            v);
        return std::formatter<std::string>::format(s, ctx);
    }
};

#endif