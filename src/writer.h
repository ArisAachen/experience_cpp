#ifndef __EXPERIENCE_SRC_WRITER_H__
#define __EXPERIENCE_SRC_WRITER_H__

#include "define.h"

#include <memory>
#include <mutex>
#include <map>
#include <sqlite3.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <condition_variable>

namespace experience {

class DataBase {
public:
    typedef std::shared_ptr<DataBase> ptr;
    typedef std::vector<std::map<std::string, Data::ptr>> Result;
    typedef std::pair<std::string, std::string> Match;
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
     * @brief close db
     */
    bool close();

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
     * @param[in] match insert data
     */
    bool insert(const std::string & table, std::map<std::string, std::string> & variant);

    /**
     * @brief 
     * @param[in] name table name
     * @param[in] match condition
     */
    bool remove(const std::string & table, const Match & match);

    /**
     * @brief read data from database
     * @param[in] table table name
     * @param[in] key key name
     * @param[in] match match condition
     * @param[out] result execute result
     * @details
     *  %int type
     *  %void* static_cast value
     */
    bool read(const std::string & table, const std::string & key, const Match & match, Result & result);

private:
    /**
     * @brief exec sql statement
     * @param[in] cmd sql statement
     */
    bool execute_no_return(const std::string & cmd);

    // TODO not good design
    /**
     * @brief exec sql statement with result
     * @param[in] cmd 
     * @param[out] result 
     * @details 
     *  %int type
     *  %void* static_cast value
     */
    bool execute_with_return(const std::string & cmd, Result & result);

private:
    // read write lock
    std::mutex mutex_;
    /// database path
    std::string path_ {""};
    /// sqlite instance
    sqlite3* db_ {nullptr};
};

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

public:
    /**
     * @brief read message from data
     */
    bool read(std::vector<std::string> & vec);

private:
    /**
     * @brief Create a table object
     * @param[in] table table name
     */
    void create_table(const std::string & table = "exp");

    /**
     * @brief Get the table object
     */
    const std::string get_table();

private:
    /// table name
    std::string table_ {""};
    /// sqlite3 instance
    DataBase::ptr db_ {nullptr};
};


class WebWriter : public WriterInterface {

};


}

#endif