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
#include "vector.h"
namespace lox {
    // this is a static amount of memory - not heap allocated
    template <typename T, size_t Max>
    class Stack {
    public:
        using value_type = T;
        void reset() {
            _top = &stack[0];
        }

        void push(T value) {
            *_top = value;
            _top++;
        }

        const T* begin() const {
            return &stack[0];
        }

        T* begin() {
            return &stack[0];
        }

        const T* end() const {
            return _top;
        }

        T* end() {
            return _top;
        }

        T pop() {
            if (_top == &stack[0]) {
                throw lox::Exception("Popping from empty stack", nullptr);
            }
            --_top;
            return *_top;
        }

        T top() const {
            if (_top == &stack[0]) {
                throw lox::Exception("Popping from empty stack", nullptr);
            }
            return *(_top - 1);
        }

    private:
        Array<T, Max> stack;
        T* _top = &stack[0];
    };

    template <typename T>
    class DynamicStack {
    public:
        using value_type = T;

        void reset() {
            stack.clear();
        }

        void push(T value) {
            stack.push_back(value);
        }

        T pop() {
            auto value = top();
            stack.eraseAt(stack.size() - 1);
            return value;
        }

        T& top() {
            if (stack.size() == 0) {
                throw lox::Exception("Can't pop from empty stack", nullptr);
            }
            return stack[stack.size() - 1];
        }

        const T& peek(size_t stackIndex = 0) const {
            if (stack.size() <= stackIndex) {
                throw lox::Exception("Can't peek from empty stack", nullptr);
            }
            return stack[stack.size() - 1 - stackIndex];
        }

        const T* begin() const {
            return stack.begin();
        }
        const T* end() const {
            return stack.end();
        }

        size_t size() const {
            return stack.size();
        }

    private:
        Vector<T> stack;
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
template <typename T>
struct std::formatter<lox::DynamicStack<T>, char> {
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FmtContext>
    FmtContext::iterator format(const lox::DynamicStack<T>& stack, FmtContext& ctx) const {
        std::ostringstream out;
        out << "        \n";
        auto valueToString = [](auto v) { return std::format("[ {} ]", v); };
        auto transformed = lox::views::transform(stack, valueToString);
        lox::ranges::copy(transformed, std::ostream_iterator<std::string>(out, "\n"));
        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

#endif