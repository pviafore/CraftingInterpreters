
#include <print>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "loxexception.h"
#include "vm.h"
int main() {
    try {
        lox::Chunk chunk;

        chunk.writeConstant(1.2, 1);
        chunk.writeConstant(3.4, 1);
        chunk.write(lox::OpCode::Add, 1);

        chunk.writeConstant(5.6, 2);
        chunk.write(lox::OpCode::Multiply, 2);
        chunk.write(lox::OpCode::Negate, 3);
        chunk.write(lox::OpCode::Return, 3);

        lox::VM vm;
        vm.diagnosticMode = true;
        vm.interpret(chunk);

        lox::printChunk(chunk, "test chunk");
    } catch (lox::BadAllocException e) {
        std::println("Bad alloc: {}", e);
    } catch (lox::Exception e) {
        std::println("Exception: {}", e);
        return 1;
    }
    return 0;
}
