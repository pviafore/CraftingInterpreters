#ifndef LOXCPP_EXCEPTION_H_
#define LOXCPP_EXCEPTION_H_

#include <iostream>
#include <source_location>
#include <stacktrace>
#include <string>

// Peter Muldoon's Omega Exception
namespace lox {
    template <typename DATA_T>
    class ExceptionBase {
    public:
        ExceptionBase(
            const char* str, const DATA_T& data,
            const std::source_location& loc = std::source_location::current(),
            std::stacktrace trace = std::stacktrace::current())
            : err_str(str), data_(data), location_{loc}, backtrace_{trace} {}
        ExceptionBase(
            std::string str, DATA_T& data,
            const std::source_location& loc = std::source_location::current(),
            std::stacktrace trace = std::stacktrace::current())
            : err_str(std::move(str)),
              data_(std::move(data)),
              location_{loc},
              backtrace_{trace} {}
        DATA_T& data() { return data_; }
        const DATA_T& data() const noexcept { return data_; }
        std::string& what() { return err_str; }
        const std::string& what() const noexcept { return err_str; }
        const std::source_location& where() const { return location_; }
        const std::stacktrace& stack() const { return backtrace_; }

    private:
        std::string err_str;
        DATA_T data_;
        const std::source_location location_;
        const std::stacktrace backtrace_;
    };

    using Exception = ExceptionBase<void*>;
    using BadAllocException = ExceptionBase<std::bad_alloc>;

}
std::ostream& operator<<(std::ostream& os, const std::stacktrace& backtrace);

template <typename T>
struct std::formatter<lox::ExceptionBase<T>, char> {
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FmtContext>
    FmtContext::iterator format(const lox::ExceptionBase<T>& exc, FmtContext& ctx) const {
        std::ostringstream out;
        auto location = exc.where();
        out << exc.what() << "\n"
            << location.file_name() << "(" << location.line() << ":"
            << location.column() << "), function `" << location.function_name() << "`\n"
            << exc.stack() << "\n";

        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};

#endif