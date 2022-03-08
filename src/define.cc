#include "define.h"


namespace experience {


void RespChainInterface::set_block(bool block) {
    // save block state
    std::unique_lock<std::mutex> lock(mutex_);
    blocked_ = block;
    // if current block is unblock, 
    // should notify
    if (!block)
        nonblock_cond_.notify_one();
}


void RespChainInterface::block() {
        // save block state
    std::unique_lock<std::mutex> lock(mutex_);
    nonblock_cond_.wait(lock, [&](){return !blocked_; });

    // block next, only all resp chain is nonblock
    // can return here
    if (next_) 
        next_->block();
}

void RespChainInterface::set_next_chain(RespChainInterface::ptr next) {
    next_ = next;
}


}