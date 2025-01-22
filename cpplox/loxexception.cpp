#include "loxexception.h"

std::ostream& operator<<(std::ostream& os, const std::stacktrace& backtrace) {
    for (auto iter = backtrace.begin(); iter != (backtrace.end() - 3); ++iter) {
        os << iter->source_file() << "(" << iter->source_line()
           << ") : " << iter->description() << "\n";
    }
    return os;
}
