
#include <print>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "loxexception.h"
int main() {
    try {
        lox::Chunk chunk;
        chunk.write(lox::OpCode::Return, 1);
        chunk.write(lox::OpCode{5}, 1);
        chunk.write(lox::OpCode::Return, 2);

        auto index = chunk.addConstant(1.2);
        chunk.write(lox::OpCode::Constant, 3);
        chunk.write(index, 3);

        lox::printChunk(chunk, "test chunk");
    } catch (lox::BadAllocException e) {
        std::println("Bad alloc: {}", e);
    } catch (lox::Exception e) {
        std::println("Exception: {}", e);
        return 1;
    }
    return 0;
}
