
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

        chunk.writeConstant(1.2, 3);
        for (int i = 4; i < 300; ++i) {
            chunk.writeConstant(2, i);
        }

        lox::printChunk(chunk, "test chunk");
    } catch (lox::BadAllocException e) {
        std::println("Bad alloc: {}", e);
    } catch (lox::Exception e) {
        std::println("Exception: {}", e);
        return 1;
    }
    return 0;
}
