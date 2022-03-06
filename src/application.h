#ifndef __EXPERIENCE_SRC_APPLICATION_H__
#define __EXPERIENCE_SRC_APPLICATION_H__

#include "define.h"

#include <memory>
#include <thread>
#include <vector>

namespace experience {


class Application {
public:
    /**
     * @brief Construct a new Application object
     */
    Application();

    /**
     * @brief Destroy the virtual Application object
     */
    virtual~Application();

    /**
     * @brief init log
     */
    void init_log();

    /**
     * @brief start application
     */
    void start();

    /**
     * @brief stop application
     */
    void stop();

private:
    /**
     * @brief init config module
     * @param vec config vector
     */
    void init_config(std::vector<ConfigInterface::ptr> vec);

    /**
     * @brief 
     * @param vec 
     */
    void init_collector(std::vector<CollectorInterface> vec);

private:
    // define thread here
    typedef std::shared_ptr<std::jthread> Thread;
    std::vector<Thread> threads_;
};

}

#endif