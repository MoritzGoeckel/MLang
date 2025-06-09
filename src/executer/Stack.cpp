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
} // namespace executor