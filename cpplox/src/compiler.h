#ifndef LOXCPP_COMPILER_H_
#define LOXCPP_COMPILER_H_

#include "string.h"
namespace lox {
    class Compiler {
    public:
        void compile(const String& s);
    };
}
#endif