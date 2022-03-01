#ifndef __EXPERIENCE_SRC_NONCOPYABLE_H__
#define __EXPERIENCE_SRC_NONCOPYABLE_H__

namespace experience {

// Noncopyable indicate class or subclass is not 
// allow copy
class Noncopyable {
public:
    // default constructor
    Noncopyable() = default;
    virtual~Noncopyable() = default;

    // dont allow copy
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
};

}

#endif