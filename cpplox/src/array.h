#ifndef CPPLOX_ARRAY_H_
#define CPPLOX_ARRAY_H_
#include <cstddef>

#include "loxexception.h"
namespace lox {
    template <typename T, size_t N>
    class Array {
    public:
        using value_type = T;
        const T& operator[](size_t index) const {
            if (index >= N) {
                throw lox::Exception("Index out of bounds for vector access", nullptr);
            }
            return data[index];
        }

        T& operator[](size_t index) {
            if (index >= N) {
                throw lox::Exception("Index out of bounds for vector access", nullptr);
            }
            return data[index];
        }

        const T* begin() const {
            return &(data[0]);
        }

        const T* end() const {
            return &(data[N]);
        }

        T* begin() {
            return &(data[0]);
        }

        T* end() {
            return &(data[N]);
        }

        size_t size() const {
            return N;
        }

    private:
        T data[N] = {T()};
    };
}
#endif