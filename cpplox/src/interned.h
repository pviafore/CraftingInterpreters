#ifndef _CPPLOX_INTERNED_H_
#define _CPPLOX_INTERNED_H_

#include "algorithm.h"
#include "string.h"

// the interned string can take a string view or a string, and will store a pointer to the string
// if it's the same string as something else, then that string will be used as an internal
namespace lox {
    class InternedString {
    public:
        InternedString() : InternedString(StringView(nullptr, nullptr)) {}
        InternedString(String str);
        InternedString(StringView sv);
        using UnderlyingString = std::variant<SharedPtr<String>, StringView>;
        struct Impl {
            UnderlyingString str;
            size_t getHash() const;

            const char* begin() const;
            const char* end() const;

            friend bool operator==(const Impl& i1, const Impl& i2) {
                auto sv1 = StringView{i1.begin(), i1.end()};
                auto sv2 = StringView{i2.begin(), i2.end()};
                return i1.getHash() == i2.getHash() && sv1 == sv2;
            };
        };

        const char* begin() const;
        const char* end() const;
        size_t size() const;
        size_t getHash() const;

        StringView string() const;

        friend bool operator==(InternedString i1, InternedString i2) {
            return ranges::is_equal(i1, i2);
        };
        friend InternedString operator+(const InternedString& i1, const InternedString& i2) {
            String s1{i1};
            s1.push_back(i2);
            return InternedString(std::move(s1));
        }

    private:
        UnderlyingString str;
    };
}
#endif