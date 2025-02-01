#ifndef LOXCPP_ITERATOR_H_
#define LOXCPP_ITERATOR_H_

#include "algorithm.h"
namespace lox {
    template <typename Container>
    class BackInsertIterator {
    public:
        using value_type = Container::value_type;
        BackInsertIterator(Container& c) : c(c) {}

        BackInsertIterator& operator*() {
            return *this;
        }

        BackInsertIterator& operator=(Container::value_type value) {
            c.push_back(value);
            return *this;
        }

        BackInsertIterator& operator++() {
            return *this;
        }

        BackInsertIterator& operator++(int) {
            return *this;
        }

    private:
        Container& c;
    };

}
#endif