#ifndef LOXCPP_OPTIONAL_H_
#define LOXCPP_OPTIONAL_H_

#include "loxexception.h"
namespace lox {
    struct Nullopt {};
    template <typename T>
    class Optional {
    public:
        Optional() : _hasValue(false) {
        }

        Optional(T t) : _hasValue(true) {
            _value = t;
        }

        ~Optional() {}

        T& value() {
            if (!_hasValue) {
                throw lox::Exception("Bad optional access", nullptr);
            }
            return _value;
        }

        const T& value() const {
            if (!_hasValue) {
                throw lox::Exception("Bad optional access", nullptr);
            }
            return _value;
        }

        operator bool() const {
            return _hasValue;
        }

        bool hasValue() const {
            return _hasValue;
        }

    private:
        T _value;

        bool _hasValue = false;
    };
}
#endif