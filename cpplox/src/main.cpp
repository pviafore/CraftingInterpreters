
#include <fstream>
#include <print>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "file.h"
#include "loxexception.h"
#include "memory.h"
#include "string.h"
#include "vm.h"

static void repl(lox::VM& vm) {
    while (true) {
        lox::String s;
        std::print("> ");
        std::cin >> s;
        if (std::cin.eof()) {
            std::println();
            break;
        }
        if (!s.empty()) {
            vm.interpret(s);
        }
    }
}

static lox::InterpretResult runFile(lox::VM& vm, const char* path) {
    lox::File file(path);
    return vm.interpret(file.contents());
}

void memtest() {
    uint32_t* p = nullptr;
    auto block1 = lox::reallocate(p, 0, 32);
    auto block2 = lox::reallocate(p, 0, 63);
    auto block3 = lox::reallocate(p, 0, 63);
    auto block4 = lox::reallocate(p, 0, 10);
    auto block5 = lox::reallocate(p, 0, 128);

    block2 = lox::reallocate(block2, 63, 64);
    block1 = lox::reallocate(block1, 32, 64);
    block4 = lox::reallocate(block4, 10, 16);
    block5 = lox::reallocate(block5, 128, 256);
    block3 = lox::reallocate(block3, 63, 128);

    block3 = lox::reallocate(block3, 128, 0);
    block5 = lox::reallocate(block5, 256, 0);
    block4 = lox::reallocate(block4, 16, 0);
    block1 = lox::reallocate(block1, 64, 0);
    block2 = lox::reallocate(block2, 64, 0);
}

int main(int argc, const char* argv[]) {
    try {
        lox::VM vm;
        if (argc == 1) {
            repl(vm);
        } else if (argc == 2) {
            if (std::string(argv[1]) == "--memtest") {
                memtest();
                return 0;
            }
            auto result = runFile(vm, argv[1]);
            return std::to_underlying(result);
        } else {
            std::println(std::cerr, "Usage: clox[path]");
        }
    } catch (lox::BadAllocException e) {
        std::println("Bad alloc: {}", e);
        return 1;
    } catch (lox::Exception e) {
        std::println("Exception: {}", e);
        return 1;
    }
    return 0;
}
