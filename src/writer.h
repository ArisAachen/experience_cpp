#ifndef __EXPERIENCE_SRC_WRITER_H__
#define __EXPERIENCE_SRC_WRITER_H__

#include "interface.h"

#include <memory>
#include <mutex>
#include <map>
#include <sqlite3.h>

namespace experience {

class DataBase {
public:
    typedef std::shared_ptr<DataBase> ptr;
    /**
     * @brief Construct a new Data Base object
     */
    DataBase(const std::string & path = database_fle);

    /**
     * @brief Destroy the virtual Data Base object
     */
    virtual~DataBase();

    /**
     * @brief open db
     */
    bool open();

    /**
     * @brief db is opened
     */
    bool is_open();

    /**
     * @brief Create a table object
     * @param[in] table_name table name
     * @param[in] typ table type
     */
    bool create_table(const std::string & name = "exp", const std::string &typ = "");
    
    /**
     * @brief insert data to table
     * @param[in] name table name 
     * @param[in] cmd insert data
     */
    bool insert(const std::string & table, std::map<std::string, std::string> && variant);

    /**
     * @brief 
     * @param[in] name 
     * @param[in] cmd 
     */
    bool remove(const std::string & table, const std::map<std::string, std::string> && variant);

    /***/
    bool read();

private:
    // execute sql command
    bool execute(const std::string & cmd);

private:
    // read write lock
    std::mutex mutex_;
    /// database path
    std::string path_ {""};
    /// sqlite instance
    sqlite3* db_ {nullptr};
};

class DBModule : public ModuleWtrCor {
public:
    typedef std::shared_ptr<DBModule> ptr;

    /**
     * @brief Destroy the virtual D B Module object
     */
    virtual~DBModule();

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

private:
    /**
     * @brief Create a table object
     * @param[in] table table name
     */
    void create_table(const std::string & table = "exp");

private:
    /// sqlite3 instance
    DataBase::ptr db_ {nullptr};
};


class WebWriter : public WriterInterface {

};


}

#endif