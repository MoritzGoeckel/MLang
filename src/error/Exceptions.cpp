#include "Exceptions.h"

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

std::string MException::show(bool underline) {
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
