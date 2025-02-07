#ifndef LOXCPP_STRING_H_
#define LOXCPP_STRING_H_
#include <cassert>
#include <cstring>
#include <format>

#include "algorithm.h"
#include "iterator.h"
#include "vector.h"
namespace lox {
    class String {
    public:
        using iterator = char*;
        using const_iterator = const char*;
        String() {}
        String(const char* data) : String(data, strlen(data)) {
        }

        String(const char* data, size_t length) {
            buffer.push_back(data, data + length);
            buffer.push_back(0);  // null terminate the string
        }
        template <sized_range Range>
        String(const Range& range) : String(range.begin(), range.size()) {}

        String(size_t count, char value = '\0') {
            buffer.reserve(count);
            lox::ranges::fill_n(BackInsertIterator(buffer), count, value);
        }

        friend bool operator==(const String& lhs, const String& rhs) {
            return lhs.buffer == rhs.buffer;
        }

        char& operator[](size_t index) {
            return buffer[index];
        }

        size_t size() const {
            return buffer.size() - 1;
        }

        const_iterator begin() const {
            return buffer.begin();
        }

        const_iterator end() const {
            return buffer.end();
        }

        iterator begin() {
            return buffer.begin();
        }

        iterator end() {
            return buffer.end();
        }

        friend std::istream& operator>>(std::istream& in, String& s) {
            char chars[1024] = {0};
            while (in.get(chars, 1024)) {
                size_t length = strlen(chars);
                s.buffer.push_back(chars, chars + length);
            }
            in.clear();
            in.ignore();
            return in;
        }

        bool empty() const {
            return buffer.size() == 0;
        }

        friend struct std::formatter<String>;

    private:
        Vector<char> buffer;
    };

    class StringView {
    public:
        using value_type = char*;
        StringView() : start(nullptr), length(0) {}
        StringView(const char* start, size_t length) : start(start), length(length) {}
        StringView(const char* start, const char* end) : start(start), length(end - start) {}
        String str() const {
            return String(start, length);
        }

        const char* begin() const {
            return start;
        }

        const char* end() const {
            return start + length;
        }

        size_t size() const {
            return length;
        }

    private:
        const char* start;
        size_t length;
    };
}
template <>
struct std::formatter<lox::String, char> {
    constexpr auto parse(auto& ctx) { return ctx.begin(); }
    auto format(const lox::String& s, auto& ctx) const {
        lox::ranges::copy(s.buffer, ctx.out());
        return ctx.out();
    }
};

template <>
struct std::formatter<lox::StringView, char> {
    template <typename ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    FormatContext::iterator format(const lox::StringView& sv, FormatContext& ctx) const {
        std::format_to(ctx.out(), "{}", lox::String(sv));
        return ctx.out();
    }
};
#endif