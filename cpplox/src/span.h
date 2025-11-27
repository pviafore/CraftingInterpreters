#ifndef LOXCPP_SPAN_H_
#define LOXCPP_SPAN_H_

#include "algorithm.h"
namespace lox {
    template <typename T>
    class Span {
    public:
        Span(const T* ptr, size_t size) : startPtr(ptr), endPtr(ptr + size) {}
        Span(const T* ptr, T* ptr2) : startPtr(ptr), endPtr(ptr2) {}
        Span(const range auto& r) : startPtr(r.begin()), endPtr(r.end()) {}
        const T* begin() const {
            return startPtr;
        }

        const T* end() const {
            return endPtr;
        }

        size_t size() const {
            return endPtr - startPtr;
        }

        using value_type = T;

    private:
        const T* startPtr = nullptr;
        const T* endPtr = nullptr;
    };

}

#endif