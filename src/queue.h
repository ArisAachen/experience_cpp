#ifndef __EXPERIENCE_SRC_QUEUE_H__
#define __EXPERIENCE_SRC_QUEUE_H__

#include "define.h"

#include <mutex>
#include <queue>
#include <condition_variable>

namespace experience {


class Queue {
public:
    /**
     * @brief Construct a new Queue object
     */
    Queue();

    /**
     * @brief Destroy the virtual Queue object
     */
    virtual~Queue();

    /**
     * @brief push request to queue
     * @param[in] msg msg
     */
    void push(ReqMessage::ptr msg);

    /**
     * @brief pop request from queue
     */
    ReqMessage::ptr pop();

    /**
     * @brief clear all queue
     */
    void clear();
    
    /**
     * @brief debug all request message
     */
    void debug();
private:
    /// request queue
    std::queue<ReqMessage::ptr> queue_;
    /// write mutex
    std::mutex mutex_;
    /// wait condition
    std::condition_variable empty_;
};





}




#endif