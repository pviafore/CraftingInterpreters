#ifndef CPPLOX_STACK_H_
#define CPPLOX_STACK_H_

#include <cstddef>
#include <format>
#include <iterator>
#include <limits>
#include <sstream>

#include "algorithm.h"
#include "array.h"
#include "value.h"
namespace lox {
    // this is a static amount of memory - not heap allocated
    template <typename T, size_t Max>
    class Stack {
    public:
        using value_type = T;
        void reset() {
            top = &stack[0];
        }

        void push(T value) {
            *top = value;
            top++;
        }

        const T* begin() const {
            return &stack[0];
        }

        T* begin() {
            return &stack[0];
        }

        const T* end() const {
            return top;
        }

        T* end() {
            return top;
        }

        T pop() {
            if (top == &stack[0]) {
                throw lox::Exception("Popping from empty stack", nullptr);
            }
            --top;
            return *top;
        }

    private:
        Array<T, Max> stack;
        T* top = &stack[0];
    };
}

template <typename T, size_t Max>
struct std::formatter<lox::Stack<T, Max>, char> {
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FmtContext>
    FmtContext::iterator format(const lox::Stack<T, Max>& stack, FmtContext& ctx) const {
        std::ostringstream out;
        out << "        \n";
        auto valueToString = [](auto v) { return std::format("[ {} ]", v); };
        auto transformed = lox::views::transform(stack, valueToString);
        lox::ranges::copy(transformed, std::ostream_iterator<std::string>(out, "\n"));
        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

#endif