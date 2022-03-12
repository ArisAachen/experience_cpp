#include "thread.h"
#include "log.h"
#include "macro.h"
#include <pthread.h>
#include <tuple>
#include <utility>


namespace experience {


template<typename T, typename... Args>
Thread<T, Args...>::Thread(const std::string name, T&& f, Args&&... args) : 
cb_(std::forward<T>(f)), args_(std::forward<Args>(args)...), name_(name)
{   
    // run thread
    // must create thread success
    EXPERIENCE_ASSERT(pthread_create(&thread_id_, nullptr, run, this) == 0);
    EXPERIENCE_FMT_DEBUG("create thread success, name: %s, thread id: %d", name.c_str() ,thread_id_);
}

template<typename T, typename... Args>
void Thread<T, Args...>::run(void* arg) {
    // set pthread can be cancelled, it is default behavoir
    EXPERIENCE_ASSERT(pthread_setcanceltype(PTHREAD_CANCEL_ENABLE, nullptr));

    // convert type
    auto thread = static_cast<Thread>(arg);
    // check type
    EXPERIENCE_ASSERT(thread != nullptr); 
    thread->cb_(std::tie(thread->args_));
    EXPERIENCE_FMT_DEBUG("thread execute end, name: %s, thread id: %d", thread->name_.c_str(), thread->thread_id_);
}

template<typename T, typename... Args>
void Thread<T, Args...>::stop() {
    // exist thread
    pthread_cancel(thread_id_);
    EXPERIENCE_FMT_DEBUG("thread cancel end, name: %s, thread id: %d", name_.c_str(), thread_id_);
}

// wait here
template<typename T, typename... Args>
Thread<T, Args...>::~Thread() {
    // thread id must be not 0
    EXPERIENCE_ASSERT(thread_id_ > 0);
    // join 
    pthread_join(thread_id_, nullptr);
}

}