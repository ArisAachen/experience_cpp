#include "thread.h"
#include "log.h"
#include "macro.h"
#include <pthread.h>
#include <tuple>
#include <utility>


namespace experience {

void* Thread::run(void* arg) {
    // set pthread can be cancelled, it is default behavoir
    EXPERIENCE_ASSERT(pthread_setcanceltype(PTHREAD_CANCEL_ENABLE, nullptr));

    // convert and run thread
    auto thread = static_cast<Thread*>(arg);
    EXPERIENCE_ASSERT(thread != nullptr);
    EXPERIENCE_FMT_DEBUG("thread execute start, thread name: %s", thread->name_.c_str());
    if (thread->base)
        thread->base->run();

    EXPERIENCE_FMT_DEBUG("thread execute end, thread name: %s", thread->name_.c_str());
    return nullptr;
}

void Thread::stop() {
    // exist thread
    pthread_cancel(thread_id_);
    EXPERIENCE_FMT_DEBUG("thread cancel end, name: %s, thread id: %d", name_.c_str(), thread_id_);
}

// wait here
Thread::~Thread() {
    // thread id must be not 0
    EXPERIENCE_ASSERT(thread_id_ > 0);
    // join 
    pthread_join(thread_id_, nullptr);
}

}