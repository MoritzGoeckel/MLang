#include "Exceptions.h"

#ifdef WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
void print_stacktrace();
 {
    const ULONG framesToSkip = 0;
    const ULONG framesToCapture = 64;
    void* backTrace[framesToCapture] {};
    ULONG backTraceHash = 0;

    const USHORT nFrame = CaptureStackBackTrace(
          framesToSkip
        , framesToCapture
        , backTrace
        , &backTraceHash
    );

    for(USHORT iFrame = 0; iFrame < nFrame; ++iFrame) {
        printf("[%3d] = %p\n", iFrame, backTrace[iFrame]);
    }
    printf("backTraceHash = %08x\n", backTraceHash);
}
#else
#include <execinfo.h> /* backtrace, backtrace_symbols_fd */
#include <unistd.h> /* STDOUT_FILENO */

void print_stacktrace() {
    size_t size;
    enum Constexpr { MAX_SIZE = 1024 };
    void *array[MAX_SIZE];
    size = backtrace(array, MAX_SIZE);
    backtrace_symbols_fd(array, size, STDOUT_FILENO);
}
#endif

MException::MException(const std::string& msg, const std::string& file,
                       int line)
    : msg(msg), file(file), line(line) {}

std::string MException::getMsg() const { return msg; }
std::string MException::getLine() const { return std::to_string(line); }
std::string MException::getFile() const { return file; }

std::string MException::getFileShort() const {
    return file.substr(file.find_last_of("/\\") + 1);
}

std::string MException::show() const {
    return msg + " in " + getFileShort() + ":" + getLine();
}

std::string MException::show(bool underline) const {
    if (underline) {
        auto banner = show();
        return banner + '\n' + std::string(banner.size(), '^');
    } else {
        return show();
    }
}

TodoException::TodoException(const char* msg, const std::string& file, int line)
    : MException(std::string("Todo: ") + msg, file, line) {}

const char* TodoException::what() const throw() { return msg.c_str(); }

ConstraintViolatedException::ConstraintViolatedException(
    const char* msg, const std::string& file, int line)
    : MException(std::string("Constraint violated: ") + msg, file, line) {}

const char* ConstraintViolatedException::what() const throw() {
    return msg.c_str();
}
