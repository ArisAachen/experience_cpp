#ifndef __EXPERIENCE_SRC_THREAD_H__
#define __EXPERIENCE_SRC_THREAD_H__


#include "log.h"
#include <string>
#include <tuple>
#include <memory>
#include <functional>
#include <pthread.h>
#include <type_traits>


namespace experience {

struct thread_data_base {
    typedef std::shared_ptr<thread_data_base> ptr;
    virtual ~thread_data_base() {}
    virtual void run() = 0;
};

template<typename Func, typename... Args>
class thread_data : public thread_data_base {
public:
    /**
     * @brief Construct a new thread data object
     * @param[in] f func 
     * @param[in] arg func params
     */
    thread_data(Func&& f, Args&&... args) :
    func_(std::forward<Func>(f)), args_(std::forward<Args>(args)...) {}

    /**
     * @brief Destroy the virtualthread data object
     */
    virtual~thread_data() {}

    /**
     * @brief run command
     */
    virtual void run() override {
        run_with_index(seq);
    }

    template<std::size_t... Index>
    void run_with_index(std::index_sequence<Index...>) {
        std::invoke(func_, std::get<Index>(args_)...);
    }

private: 
    /// func pointer
    Func func_ {nullptr};
    /// arguments
    std::tuple<typename std::decay<Args>::type...> args_;
    /// index sequence
    std::index_sequence_for<Args...> seq {};
};

class Thread {
    public:
    /**
     * @brief delete construct a new Thread object
     */
    Thread() = delete;
    /**
     * @brief Construct a new Thread object
     * @param[in] name thread op name
     * @param[in] f func pointer
     * @param[in] args arguments
     */
    template<typename Func, typename... Args>
    Thread(const std::string & name, Func&& f, Args&&... args) : 
        name_(name), base(new thread_data<Func, Args...>(std::forward<Func>(f), std::forward<Args>(args)...))
    {
        EXPERIENCE_FMT_DEBUG("create thread, name: %s", name_.c_str());
        pthread_create(&thread_id_, nullptr, run, this);
    }

    /**
     * @brief Destroy the virtual Thread object
     */
    virtual~Thread();

    /**
     * @brief stop thread
     */
    void stop();

private:
    /**
     * @brief run thread
     * 
     * @param[in] arg this thread 
     */
    static void* run(void* arg);

private:
    /// thread name
    std::string name_ {""};
    /// base pointer
    thread_data_base::ptr base;
    /// thread id
    pthread_t thread_id_ {0};
};

}


#endif