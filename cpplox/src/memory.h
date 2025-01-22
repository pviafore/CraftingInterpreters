#ifndef CLOXCPP_MEMORY_H_
#define CLOXCPP_MEMORY_H_

#include <new>

#include "common.h"
#include "loxexception.h"

namespace lox {
    template <typename T>
    T *reallocate(T *pointer, size_t, size_t newSize) {
        if (newSize == 0) {
            free(pointer);
            return nullptr;
        }

        T *result = (T *)realloc(pointer, newSize);
        if (!result)
            throw BadAllocException{"Memory realloc failed", std::bad_alloc{}};
        return result;
    }
}
#endif