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

    NativeFunction::NativeFunction(Func f, size_t argCount) : function(f), argCount(argCount) {}

    Expected<Value, String> NativeFunction::invoke(size_t args, Span<Value> values) {
        if (args != this->argCount) {
            return String{"Wrong number of arguments"};
        }
        return function(values);
    }
}