#ifndef CLOXCPP_VECTOR_H_
#define CLOXCPP_VECTOR_H_

#include <algorithm>

#include "loxexception.h"
#include "memory.h"
namespace lox {
    template <typename T>
    class Vector {
    public:
        using iterator = T*;
        using const_iterator = const T*;
        Vector() {}

        ~Vector() { reallocate(data, sizeof(T) * count, 0); }

        Vector(const Vector& rhs) = delete;
        Vector& operator=(const Vector& rhs) = delete;

        Vector(Vector&& rhs) {
            std::swap(data, rhs.data);
            std::swap(count, rhs.count);
            std::swap(capacity, rhs.capacity);
        }
        Vector& operator=(Vector&& rhs) {
            if (this == &rhs) {
                return *this;
            }
            this->data = rhs.data;
            this->count = rhs.count;
            this->capacity = rhs.capacity;
            std::swap(Vector{}, rhs);
        }

        T operator[](size_t index) const {
            if (index >= count) {
                throw lox::Exception("Index out of bounds for vector access", nullptr);
            }
            return this->data[index];
        }
        T& operator[](size_t index) {
            if (index >= count) {
                throw lox::Exception("Index out of bounds for vector access", nullptr);
            }
            return this->data[index];
        }

        void clear() {
            reallocate(data, sizeof(T) * count, 0);
            count = 0;
            capacity = 0;
        }

        const T* begin() const {
            return data;
        }

        const T* end() const {
            return data + count;
        }

        size_t size() const {
            return count;
        }

        void push_back(T value) {
            if (capacity < count + 1) {
                size_t newCapacity = std::max(8uz, capacity * 2);
                data =
                    (T*)reallocate(data, sizeof(T) * capacity, sizeof(T) * newCapacity);
                capacity = newCapacity;
            }

            data[count++] = value;
        }

    private:
        T* data = nullptr;
        size_t count = 0;
        size_t capacity = 0;
    };

}  // namespace lox
#endif