#include "file.h"

#include "loxexception.h"
namespace lox {
    File::File(const char* path) : file(fopen(path, "rb")) {
        if (file == nullptr) {
            throw lox::Exception("Could not open file", nullptr);
        }
    }

    File::~File() {
        fclose(file);
    }

    String File::contents() {
        if (read) {
            return s;
        }
        fseek(file, 0L, SEEK_END);
        size_t filesize = ftell(file);
        rewind(file);

        s = String(filesize + 1);
        size_t bytesread = fread(&s[0], sizeof(char), filesize, file);
        if (bytesread < filesize) {
            throw lox::Exception("Could not read enough of the file", nullptr);
        }

        read = true;
        return s;
    }
}