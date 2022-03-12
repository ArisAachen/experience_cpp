#ifndef __EXPERIENCE_SRC_THREAD_H__
#define __EXPERIENCE_SRC_THREAD_H__


#include <string>
#include <tuple>
#include <functional>
#include <pthread.h>
#include <type_traits>


namespace experience {

template<typename T, typename... Args>
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
    Thread(const std::string name, T&& f, Args&&... args);

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
    static void run(void* arg);

private:
    /// thread name
    std::string name_ {""};
    /// func
    T cb_ {nullptr};
    /// arguments
    std::tuple<typename std::decay<Args>::type...> args_;
    /// thread id
    pthread_t thread_id_ {0};
};

}


#endif