#ifndef CPPLOX_UTILITY_H_
#define CPPLOX_UTILITY_H_

namespace lox {
    template <typename F, typename S>
    struct Pair {
        F first;
        S second;

        friend auto operator<=>(const Pair& lhs, const Pair& rhs) = default;
    };
}
#endif