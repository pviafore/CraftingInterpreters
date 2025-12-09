#include "object.h"

#include "algorithm.h"
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

    void Function::setName(StringView name) {
        this->name = name;
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

    size_t Function::addUpvalue(size_t index, bool isLocal) {
        auto size = upvalues.size();
        if (size == std::numeric_limits<uint8_t>::max()) {
            throw Exception("Too many closure variables in a function", nullptr);
        }
        upvalues.push_back({isLocal, index});
        return size;
    }

    Optional<size_t> Function::getUpvalue(size_t index, bool isLocal) {
        for (const auto& [i, item] : views::enumerate(upvalues)) {
            if (item.index == index && item.isLocal == isLocal) {
                return i;
            }
        }
        return {};
    }

    const Vector<Function::UpValue>& Function::getUpvalues() const {
        return upvalues;
    }

    size_t Function::getUpValueCount() const {
        return upvalues.size();
    }

    NativeFunction::NativeFunction(Func f, size_t argCount) : function(f), argCount(argCount) {}

    Expected<Value, String> NativeFunction::invoke(size_t args, Span<Value> values) {
        if (args != this->argCount) {
            return String{"Wrong number of arguments"};
        }
        return function(values);
    }

    Closure::Closure(SharedPtr<Function> f) : f(f) {}
    SharedPtr<Function> Closure::getFunction() const { return f; }
    void Closure::addUpValue(SharedPtr<UpValueObj> obj) {
        upvalues.push_back(obj);
    }

    SharedPtr<UpValueObj> Closure::getUpValue(size_t index) const {
        return upvalues[index];
    }
    void Closure::setUpValue(size_t index, Value value) {
        *(upvalues[index]->location) = value;
    }

    Class::Class(InternedString name) : name(name) {}
    StringView Class::getName() const { return name.string(); }

    Instance::Instance(SharedPtr<Class> cls) : cls(cls) {}
    StringView Instance::getName() const { return cls->getName(); }

    Optional<Value> Instance::getField(InternedString name) const {
        return fields.get(name);
    }

    void Instance::setField(InternedString name, Value v) {
        fields.insert(name, v);
    }
}