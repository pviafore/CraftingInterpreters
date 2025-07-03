#include "interned.h"

#include "table.h"

namespace lox {

    static auto& getStrings() {
        static HashSet<InternedString::Impl> strings;
        return strings;
    }
    InternedString::InternedString(String s) {
        UnderlyingString underlying = SharedPtr<String>::Make(std::move(s));
        Impl impl{underlying};
        str = getStrings().insert(std::move(impl)).str;
    }

    InternedString::InternedString(StringView sv) {
        str = getStrings().insert({sv}).str;
    }

    size_t InternedString::size() const {
        if (std::holds_alternative<StringView>(str)) {
            return std::get<StringView>(str).size();
        } else {
            return std::get<SharedPtr<String>>(str)->size();
        }
    }

    size_t getHash(const InternedString::UnderlyingString& us) {
        if (std::holds_alternative<StringView>(us)) {
            return std::get<StringView>(us).getHash();
        } else {
            return std::get<SharedPtr<String>>(us)->getHash();
        }
    }

    size_t InternedString::Impl::getHash() const {
        return lox::getHash(str);
    }

    size_t InternedString::getHash() const {
        return lox::getHash(str);
    }

    StringView InternedString::string() const {
        return StringView{begin(), end()};
    }

    const char* begin(const InternedString::UnderlyingString& us) {
        if (std::holds_alternative<StringView>(us)) {
            return std::get<StringView>(us).begin();
        } else {
            return std::get<SharedPtr<String>>(us)->begin();
        }
    }
    const char* end(const InternedString::UnderlyingString& us) {
        if (std::holds_alternative<StringView>(us)) {
            return std::get<StringView>(us).end();
        } else {
            return std::get<SharedPtr<String>>(us)->end();
        }
    }

    const char* InternedString::Impl::begin() const {
        return lox::begin(str);
    }
    const char* InternedString::Impl::end() const {
        return lox::end(str);
    }

    const char* InternedString::begin() const {
        return lox::begin(str);
    }

    const char* InternedString::end() const {
        return lox::end(str);
    }
}