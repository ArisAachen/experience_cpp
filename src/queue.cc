#include "queue.h"
#include "define.h"
#include <cstddef>
#include <mutex>
#include <queue>

namespace experience {

Queue::Queue() {

}

Queue::~Queue() {
    // clear queue
    clear();
}

// push req
void Queue::push(ReqMessage::ptr msg) {
    {
        // lock here
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(msg);
    }
    // notify current is not empty
    empty_.notify_one();
}

// pop request 
ReqMessage::ptr Queue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    // wait until queue is not empty
    empty_.wait(lock, [&](){ return !queue_.empty(); });
    // get element
    auto elem = queue_.front();
    queue_.pop();
    return elem;
}

// clear all queue
void Queue::clear() {
    std::queue<ReqMessage::ptr> empty;
    queue_.swap(empty);
}

void Queue::debug() {

}

}