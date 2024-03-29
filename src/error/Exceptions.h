#pragma once

#include <exception>
#include <string>

struct MException : public std::exception {
    MException(const std::string& msg, const std::string& file, int line);

    std::string getMsg() const;
    std::string getLine() const;
    std::string getFile() const;
    std::string getFileShort() const;

    virtual std::string show() const;
    virtual std::string show(bool underline);

   protected:
    std::string msg;
    std::string file;
    int line;
};

struct TodoException : public MException {
    TodoException(const char* msg, const std::string& file, int line);

    const char* what() const throw();
};
#define throwTodo(msg) throw TodoException(msg, __FILE__, __LINE__);

struct ConstraintViolatedException : public MException {
    ConstraintViolatedException(const char* msg, const std::string& file,
                                int line);

    const char* what() const throw();
};
#define throwConstraintViolated(msg) \
    throw ConstraintViolatedException(msg, __FILE__, __LINE__);
