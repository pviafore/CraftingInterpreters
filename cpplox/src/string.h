#ifndef LOXCPP_STRING_H_
#define LOXCPP_STRING_H_
#include <cassert>
#include <cstring>
#include <format>

#include "algorithm.h"
#include "iterator.h"
#include "span.h"
#include "vector.h"
namespace lox {
    size_t getHash(Span<char> s);
    class String {
    public:
        using iterator = char*;
        using const_iterator = const char*;
        String() {
            buffer.push_back(0);
            hashString();
        }
        String(const char* data) : String(data, strlen(data)) {
        }

        String(const char* data, size_t length) {
            buffer.push_back(data, data + length);
            buffer.push_back(0);  // null terminate the string
            hashString();
        }

        String(const char* start, const char* end) : String(start, end - start) {}

        template <sized_range Range>
        String(const Range& range) : String(range.begin(), range.size()) {}

        String(size_t count, char value = '\0') {
            buffer.reserve(count);
            lox::ranges::fill_n(BackInsertIterator(buffer), count, value);
            buffer.push_back(0);
            hashString();
        }

        friend bool operator==(const String& lhs, const String& rhs) {
            if (lhs.hash != rhs.hash) {
                return false;
            }
            return lhs.buffer == rhs.buffer;
        }

        friend String operator+(const String& lhs, const String& rhs) {
            String out = lhs;
            out.push_back(rhs);
            return out;
        }

        char operator[](size_t index) const {
            return buffer[index];
        }

        size_t size() const {
            return buffer.size() - 1;
        }

        const_iterator begin() const {
            return buffer.begin();
        }

        // do not include the null terminator
        const_iterator end() const {
            return buffer.end() - 1;
        }

        iterator begin() {
            return buffer.begin();
        }

        // do not include the null terminator
        iterator end() {
            return buffer.end() - 1;
        }

        friend std::istream& operator>>(std::istream& in, String& s) {
            char chars[1024] = {0};
            s.buffer.pop_back();  // pop the null terminating character
            while (in.get(chars, 1024)) {
                size_t length = strlen(chars);
                s.buffer.push_back(chars, chars + length);
            }
            s.buffer.push_back(0);
            in.clear();
            in.ignore();
            return in;
        }

        bool empty() const {
            return buffer.size() == 0;
        }

        const char* c_str() const {
            return &buffer[0];
        }

        size_t getHash() const {
            return hash;
        }

        template <range Range>
        void push_back(const Range& range) {
            buffer.pop_back();  // pop the null terminating character
            buffer.push_back(range.begin(), range.end());
            buffer.push_back(0);
            hashString();
        }

        friend struct std::formatter<String>;

    private:
        void hashString() {
            hash = lox::getHash(buffer);
        }

        size_t hash = 0;
        Vector<char> buffer;
    };

    class StringView {
    public:
        using value_type = char*;
        StringView() : start(nullptr), length(0) {}
        StringView(const char* start, size_t length) : start(start), length(length) {}
        StringView(const char* start, const char* end) : StringView(start, end - start) {}
        StringView(const char* start) : StringView(start, strlen(start)) {}
        StringView(const String& s) : StringView(s.begin(), s.end()) {}
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
        size_t getHash() const {
            return lox::getHash({start, length});
        }

        friend bool operator==(const StringView& sv1, const StringView& sv2) {
            return ranges::is_equal(sv1, sv2);
        }

    private:
        const char* start;
        size_t length;
    };
}
template <>
struct std::formatter<lox::String> : std::formatter<std::string> {
    constexpr auto parse(auto& ctx) {
        return std::formatter<std::string>{}.parse(ctx);
    }
    auto format(const lox::String& s, auto& ctx) const {
        return std::formatter<std::string>{}.format(std::string(s.begin(), s.end()), ctx);
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