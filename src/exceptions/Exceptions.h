#pragma once

#include <exception>
#include <string>

struct MException : public std::exception {
    MException(const std::string& msg, const std::string& file, int line)
        : msg(msg), file(file), line(line) {}

    std::string getMsg() const { return msg; }
    std::string getLine() const { return std::to_string(line); }
    std::string getFile() const { return file; }
    std::string getFileShort() const {
        return file.substr(file.find_last_of("/\\") + 1);
    }

    virtual std::string show() const {
        return msg + " in " + getFileShort() + ":" + getLine();
    }

    virtual std::string show(bool underline) {
        if (underline) {
            auto banner = show();
            return banner + '\n' + std::string(banner.size(), '^');
        } else {
            return show();
        }
    }

   protected:
    std::string msg;
    std::string file;
    int line;
};

struct TodoException : public MException {
    TodoException(const char* msg, const std::string& file, int line)
        : MException(std::string("Todo: ") + msg, file, line) {}
    const char* what() const throw() { return msg.c_str(); }
};
#define throwTodo(msg) throw TodoException(msg, __FILE__, __LINE__);

struct ConstraintViolatedException : public MException {
    ConstraintViolatedException(const char* msg, const std::string& file,
                                int line)
        : MException(std::string("Constraint violated: ") + msg, file, line) {}
    const char* what() const throw() { return msg.c_str(); }
};
#define throwConstraintViolated(msg) \
    throw ConstraintViolatedException(msg, __FILE__, __LINE__);
