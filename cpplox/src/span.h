#ifndef LOXCPP_SPAN_H_
#define LOXCPP_SPAN_H_

namespace lox {
    template <typename T>
    class Span {
    public:
        Span(const T* ptr, size_t size) : startPtr(ptr), endPtr(ptr + size) {}
        Span(const T* ptr, T* ptr2) : startPtr(ptr), endPtr(ptr2) {}
        const T* begin() const {
            return startPtr;
        }

        const T* end() const {
            return endPtr;
        }

        using value_type = T;

    private:
        const T* startPtr = nullptr;
        const T* endPtr = nullptr;
    };

}

#endif