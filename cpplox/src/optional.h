#ifndef LOXCPP_OPTIONAL_H_
#define LOXCPP_OPTIONAL_H_

#include "loxexception.h"
namespace lox {
    struct Nullopt {};
    template <typename T>
    class Optional {
    public:
        Optional() : _hasValue(false) {
            _value.n = Nullopt{};
        }

        Optional(T t) : _hasValue(true) {
            _value.t = t;
        }

        T value() const {
            if (!_hasValue) {
                throw lox::Exception("Bad optional access", nullptr);
            }
            return _value.t;
        }

        operator bool() const {
            return _hasValue;
        }

        bool hasValue() const {
            return _hasValue;
        }

    private:
        union Value {
            Nullopt n;
            T t;
        };
        Value _value;

        bool _hasValue = false;
    };
}
#endif