#pragma once

#include <vector>
#include "../error/Exceptions.h"
#include "Types.h"

namespace executor {
    
class Stack {
private:
    std::vector<word_t> impl;
public:
    void push(word_t value);

    word_t pop();

    word_t back() const;

    bool empty() const;

    size_t size() const;

    word_t lookback(size_t n) const;

    // Indexed access for register-based VM
    word_t get(size_t index) const;
    void set(size_t index, word_t value);
    word_t& operator[](size_t index);
    const word_t& operator[](size_t index) const;

    using const_iterator = std::vector<word_t>::const_iterator;

    auto begin() const {
        return impl.begin();
    }

    auto end() const {
        return impl.end();
    }
};

} // namespace executor