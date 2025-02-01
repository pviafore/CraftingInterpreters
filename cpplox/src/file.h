#ifndef LOXCPP_FILE_H_
#define LOXCPP_FILE_H_
#include <cstdio>

#include "string.h"
namespace lox {
    class File {
    public:
        File(const char* path);
        ~File();
        String contents();

    private:
        FILE* file;
        bool read = false;
        String s;
    };
}

#endif