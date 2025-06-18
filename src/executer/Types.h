#pragma once

namespace executor {

#ifdef WIN
using word_t = unsigned long long;
#else
using word_t = unsigned long;
#endif

static_assert(sizeof(word_t) == 8);

} // namespace executor