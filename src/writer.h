#ifndef __EXPERIENCE_SRC_WRITER_H__
#define __EXPERIENCE_SRC_WRITER_H__

#include "interface.h"
#include <memory>


namespace experience {

class DBModule : public ModuleWtrCor {
public:
    typedef std::shared_ptr<DBModule> ptr;

    /**
     * @brief try to connect to url path
     * @param[in] url connect path
     */
    virtual void connect(const std::string & url) override;

    /**
     * @brief disconnect from server
     */
    virtual void disconnect() override;

    /**
     * @brief write queue data to writer
     * @param[in] que request queue
     */
    virtual void write(QueueInterface::ptr que) override;

    /**
     * @brief ollect message to queue
     * @param[in] que collect queue
     */
    virtual void collect(QueueInterface::ptr que) override;

    /**
     * @brief handle request result
     * @param[in] result req result
     */
    virtual void handler(ReqResult::ptr result) override;
};


class WebWriter : public WriterInterface {

}


}

#endif