
#include <fstream>
#include <print>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "file.h"
#include "loxexception.h"
#include "string.h"
#include "vm.h"

static void repl(lox::VM& vm) {
    lox::String s;
    while (true) {
        std::print("> ");
        std::cin >> s;
        if (s.empty()) {
            std::println();
            break;
        }
        vm.interpret(s);
    }
}

static lox::InterpretResult runFile(lox::VM& vm, const char* path) {
    lox::File file(path);
    return vm.interpret(file.contents());
}

int main(int argc, const char* argv[]) {
    try {
        lox::VM vm;
        if (argc == 1) {
            repl(vm);
        } else if (argc == 2) {
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
