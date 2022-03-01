#ifndef __EXPERIENCE_SRC_MACRO_H__
#define __EXPERIENCE_SRC_MACRO_H__

#include <cassert>


namespace experience {

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define EXPERIENCE_ASSERT(x) \
    if (unlikely(x)) \
        assert(x)
}

#endif