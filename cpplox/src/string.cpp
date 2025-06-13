#include "string.h"

namespace lox {
    size_t getHash(Span<char> s) {
        size_t hash = 2166136261u;
        for (auto c : s) {
            hash ^= c;
        }
        hash *= 16777619;
        return hash;
    }
}