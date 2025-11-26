#ifndef LOXCPP_OBJECT_H_
#define LOXCPP_OBJECT_H_

#include <any>

#include "memory.h"
#include "string.h"
namespace lox {
    class Chunk;

    class Function {
    public:
        Function(StringView name);
        ~Function();

        SharedPtr<Chunk> getChunk() const;
        StringView getName() const;
        uint8_t getArity() const;

        friend bool operator==(const Function& f1, const Function& f2) {
            return &f1 == &f2;
        }

        void increaseArity();

    private:
        uint8_t arity = 0;
        StringView name;
        SharedPtr<Chunk> chunk;
    };

}
#endif