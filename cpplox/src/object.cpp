#include "object.h"

#include "chunk.h"
namespace lox {
    Function::Function(StringView name) : name(name), chunk(SharedPtr<Chunk>::Make()) {}
    Function::~Function() {}
    SharedPtr<Chunk> Function::getChunk() const {
        return chunk;
    }
    StringView Function::getName() const {
        return name;
    }

    void Function::increaseArity() {
        if (arity == 255) {
            throw Exception("Can't have more than 255 parameters", nullptr);
        }
        arity++;
    }

    uint8_t Function::getArity() const {
        return arity;
    }

    NativeFunction::NativeFunction(std::function<Value(int, Span<Value>)> f) : function(f) {}

    Value NativeFunction::invoke(int argCount, Span<Value> values) {
        return function(argCount, values);
    }
}