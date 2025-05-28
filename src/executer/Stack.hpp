#pragma once

namespace executor {

// TODO: Duplicate of src/executer/ByteCode.h
using word_t = unsigned long;
    
class Stack {
private:
    std::vector<word_t> impl;
public:
    void push(word_t value) {
        impl.push_back(value);
    }

    word_t pop() {
        if (impl.empty()) {
            throw std::runtime_error("Stack underflow");
        }
        word_t value = impl.back();
        impl.pop_back();
        return value;
    }

    word_t back() const {
        if (impl.empty()) {
            throw std::runtime_error("Stack is empty");
        }
        return impl.back();
    }

    bool empty() const {
        return impl.empty();
    }

    size_t size() const {
        return impl.size();
    }

    using const_iterator = std::vector<word_t>::const_iterator;

    auto begin() const {
        return impl.begin();
    }

    auto end() const {
        return impl.end();
    }
};

} // namespace executor