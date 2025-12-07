#include <variant>
namespace lox {
    template <typename T, typename E>
    class Expected {
    public:
        Expected(T t) : _value(t) {}
        Expected(E e) : _value(e) {}

        bool hasValue() const {
            return std::holds_alternative<T>(_value);
        }

        const T& value() const {
            if (!hasValue()) {
                throw Exception("Expected had error instead of value", nullptr);
            }
            return std::get<T>(_value);
        }

        const E& error() const {
            if (hasValue()) {
                throw Exception("Expected had value instead of error", nullptr);
            }
            return std::get<E>(_value);
        }

    private:
        std::variant<T, E> _value;
    };
}