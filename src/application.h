#ifndef __EXPERIENCE_SRC_APPLICATION_H__
#define __EXPERIENCE_SRC_APPLICATION_H__

#include "define.h"

#include <vector>

namespace experience {


class Application {
public:
    /**
     * @brief Construct a new Application object
     */
    Application();

    /**
     * @brief init log
     */
    void init_log();

    /***/
    void start();

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
};

}

#endif