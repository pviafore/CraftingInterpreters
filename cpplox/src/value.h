#ifndef CPPLOX_VALUE_H_
#define CPPLOX_VALUE_H_

#include <format>
#include <variant>
namespace lox {
    using Value = std::variant<bool, std::nullptr_t, double>;

    inline bool isFalsey(Value value) {
        return std::holds_alternative<nullptr_t>(value) || (std::holds_alternative<bool>(value) && !std::get<bool>(value));
    }
}

template <class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

template <>
struct std::formatter<lox::Value, char> {
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FormatContext>
    FormatContext::iterator format(const lox::Value& v, FormatContext& ctx) const {
        using namespace std::string_literals;
        auto s = std::visit(
            overload{
                [&ctx](nullptr_t) { return "nil"s; },
                [&ctx](auto v) { return std::to_string(v); }},
            v);
        return std::ranges::copy(s, ctx.out()).out;
    }
};

#endif