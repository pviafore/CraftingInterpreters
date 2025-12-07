#ifndef LOXCPP_OBJECT_H_
#define LOXCPP_OBJECT_H_

#include <any>

#include "memory.h"
#include "stack.h"
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
        size_t addUpvalue(size_t index, bool isLocal);
        Optional<size_t> getUpvalue(size_t index, bool isLocal);
        size_t getUpValueCount() const;

        struct UpValue {
            bool isLocal = false;
            size_t index = 0;
        };
        const Vector<UpValue>& getUpvalues() const;

    private:
        uint8_t arity = 0;
        StringView name;
        SharedPtr<Chunk> chunk;
        Vector<UpValue> upvalues;
    };

}
#endif