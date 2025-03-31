#ifndef CLOXCPP_VECTOR_H_
#define CLOXCPP_VECTOR_H_

#include <cstring>

#include "algorithm.h"
#include "loxexception.h"
#include "memory.h"
namespace lox {
    template <typename T>
    class Vector {
    public:
        using iterator = T*;
        using const_iterator = const T*;
        using value_type = T;
        Vector() {}

        ~Vector() {
            if (capacity) {
                data = reallocate(data, sizeof(T) * capacity, 0);
            }
        }

        Vector(const Vector& rhs) {
            data = reallocate(data, 0, sizeof(T) * rhs.capacity);
            auto src = lox::ranges::from_buffer(rhs.data, rhs.count);
            lox::ranges::copy(src, data);
            capacity = rhs.capacity;
            count = rhs.count;
        };
        Vector& operator=(const Vector& rhs) {
            data = reallocate(data, sizeof(T) * capacity, sizeof(T) * rhs.capacity);
            auto src = lox::ranges::from_buffer(rhs.data, rhs.count);
            lox::ranges::copy(src, data);
            capacity = rhs.capacity;
            count = rhs.count;
            return *this;
        }

        Vector(Vector&& rhs) {
            data = rhs.data;
            count = rhs.count;
            capacity = rhs.capacity;
            rhs.data = nullptr;
            rhs.count = 0;
            rhs.capacity = 0;
        }
        Vector& operator=(Vector&& rhs) {
            if (this == &rhs) {
                return *this;
            }
            this->data = rhs.data;
            this->count = rhs.count;
            this->capacity = rhs.capacity;
            rhs.data = nullptr;
            rhs.count = 0;
            rhs.capacity = 0;
            return *this;
        }

        void reserve(size_t newCapacity) {
            if (newCapacity > capacity) {
                data = reallocate(data, sizeof(T) * capacity, sizeof(T) * newCapacity);
                if (data == nullptr) {
                    throw lox::Exception("Could not allocate enough memory", nullptr);
                }
                this->capacity = newCapacity;
            }
        }

        const T& operator[](size_t index) const {
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
            reallocate(data, sizeof(T) * capacity, 0);
            data = nullptr;
            count = 0;
            capacity = 0;
        }

        const T* begin() const {
            return data;
        }

        const T* end() const {
            return data + count;
        }

        T* begin() {
            return data;
        }

        T* end() {
            return data + count;
        }

        size_t size() const {
            return count;
        }

        void push_back(T value) {
            push_back(&value, (&value) + 1);
        }

        // assumes that we can ptrdiff it
        void push_back(const T* begin, const T* end) {
            auto requestedSize = end - begin;
            size_t newCapacity = capacity;
            while (newCapacity < count + requestedSize) {
                newCapacity = std::max(8uz, newCapacity * 2);
            }
            if (newCapacity != capacity) {
                data =
                    (T*)reallocate(data, sizeof(T) * capacity, sizeof(T) * newCapacity);
                capacity = newCapacity;
            }

            lox::ranges::copy(lox::ranges::from_buffer(begin, requestedSize), data + count);
            count += requestedSize;
        }

        void pop_back() {
            eraseAt(size() - 1);
        }

        void eraseAt(size_t index) {
            if (index >= count) {
                throw lox::Exception("Can't erase past our usual index", nullptr);
            }
            for (; index < count - 1; ++index) {
                data[index] = data[index - 1];
            }
            count--;
            // shrink where we need to. I don't want to go by two becuase
            // we would thrash at boundaries
            if (capacity >= 8 && count < capacity / 4 && count > 8) {
                data = reallocate(data, sizeof(T) * capacity, sizeof(T) * capacity / 2);
                capacity /= 2;
            }
        }

        friend bool operator==(const Vector<T>& lhs, const Vector<T>& rhs) {
            return lhs.count == rhs.count && memcmp(lhs.data, rhs.data, lhs.count * sizeof(T)) == 0;
        }

    private:
        T* data = nullptr;
        size_t count = 0;
        size_t capacity = 0;
    };

}  // namespace lox

#endif