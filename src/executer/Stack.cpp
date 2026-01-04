#include "Stack.h"

#include "../error/Exceptions.h"

namespace executor {
void Stack::push(word_t value) {
    impl.push_back(value);
}

word_t Stack::pop() {
    if (impl.empty()) {
        throwConstraintViolated("Cannot pop from an empty stack");
    }
    word_t value = impl.back();
    impl.pop_back();
    return value;
}

word_t Stack::back() const {
    if (impl.empty()) {
        throwConstraintViolated("Cannot access back of an empty stack");
    }
    return impl.back();
}


word_t Stack::lookback(size_t n) const{
    if (n >= impl.size()) {
        throwConstraintViolated("Lookback index out of bounds");
    }
    return impl[impl.size() - 1 - n];
}

bool Stack::empty() const {
    return impl.empty();
}

size_t Stack::size() const {
    return impl.size();
}

word_t Stack::get(size_t index) const {
    if (index >= impl.size()) {
        throwConstraintViolated("Stack::get index out of bounds");
    }
    return impl[index];
}

void Stack::set(size_t index, word_t value) {
    if (index >= impl.size()) {
        throwConstraintViolated("Stack::set index out of bounds");
    }
    impl[index] = value;
}

word_t& Stack::operator[](size_t index) {
    if (index >= impl.size()) {
        throwConstraintViolated("Stack::operator[] index out of bounds");
    }
    return impl[index];
}

const word_t& Stack::operator[](size_t index) const {
    if (index >= impl.size()) {
        throwConstraintViolated("Stack::operator[] const index out of bounds");
    }
    return impl[index];
}
} // namespace executor