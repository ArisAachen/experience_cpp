#include "writer.h"
#include "define.h"
#include "log.h"
#include "macro.h"
#include "utils.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sqlite3.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace experience {

DataBase::DataBase(const std::string & path) {
    // save and check
    path_ = path;
    EXPERIENCE_ASSERT(db_ == nullptr);
}

DataBase::~DataBase() {
    close();
}

// open database
bool DataBase::open() {
    try {
        // database should open, if not exist should create one
        if (sqlite3_open_v2(path_.c_str(), &db_, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, nullptr) != SQLITE_OK) 
            throw std::logic_error("sqlite3 operation failed");

        // open database successfully
        EXPERIENCE_FMT_INFO("database open successfully, path: %s", path_.c_str());
    } catch(std::exception & exp) {
        EXPERIENCE_FMT_ERR("database open failed, err: %s", exp.what());
        return false;
    }
    return true;
}

bool DataBase::close() {
    sqlite3_close_v2(db_);
    db_ = nullptr;
    return true;
}

// check if database is opened
bool DataBase::is_open() {
    return db_ != nullptr;
}

// create table
bool DataBase::create_table(const std::string & name, const std::string &typ) {
    // create table command
    std::string cmd = "CREATE TABLE IF NOT EXIST " + name + "(" + typ + ");";
    if (!execute_no_return(cmd)) {
        EXPERIENCE_FMT_ERR("create table %s failed", name.c_str());
        return false;
    }
    EXPERIENCE_FMT_INFO("create table %s successfully", name.c_str());
    return true;
}

// insert data to table
bool DataBase::insert(const std::string & table, std::map<std::string, std::string> & variant) {
    // check if database is opened
    if (!is_open()) {
        EXPERIENCE_FMT_ERR("insert data into table %s failed, database is not yet open", table.c_str());
        return false;
    }
    // vec define
    std::vector<std::string> keys;
    std::vector<std::string> values;
    // append
    for (auto iter : variant) {
        values.emplace_back(iter.second);
        keys.emplace_back(iter.first);
    }
    // convert key and value
    std::string db_key = StringUtils::join(keys, ",");
    std::string db_value = StringUtils::join(values, ",");
    // sql statement
    std::string statement("insert into ");
    // append table name
    statement.append(table);
    statement.append("(" + db_key + ")" + " ");
    statement.append("values (" + db_value + ");");
    // execute statement
    if (!execute_no_return(statement)) {
        EXPERIENCE_FMT_ERR("insert data %s to table %s failed", table.c_str(), statement.c_str());
        return false;
    }
    EXPERIENCE_FMT_ERR("insert data %s to table %s successfully", table.c_str(), statement.c_str());
    return true;
}

// remove data according to key
bool DataBase::remove(const std::string &table, const Match & match) {
    // check if database is opened
    if (!is_open()) {
        EXPERIENCE_FMT_ERR("delete data from table %s failed, database is not yet open", table.c_str());
        return false;
    }
    std::string statement("delete from ");
    statement.append(table);
    statement.append(" where " + match.first + "=" + match.second + ";");
    // execute statement
    if (!execute_no_return(statement)) {
        EXPERIENCE_FMT_ERR("delete data %s from table %s failed", table.c_str(), statement.c_str());
        return false;
    }
    EXPERIENCE_FMT_ERR("delete data %s from table %s successfully", table.c_str(), statement.c_str());
    return true;    
}

// read data from 
bool DataBase::read(const std::string & table, const std::string & key, const Match & match, Result & result) {
    // check if database is opened
    if (!is_open()) {
        EXPERIENCE_FMT_ERR("read data from table %s failed, database is not yet open", table.c_str());
        return false;
    }    
    // lock 
    std::lock_guard<std::mutex> lock(mutex_);
    sqlite3_stmt* pStmt;
    std::string statement("select " + key + " from " + table);
    // extra data here
    if (match.first != "") {
        statement.append(" where " + match.first + "=" + match.second);
    }
    // end
    statement.append(";");
    // read data from database
    return execute_with_return(statement, result);
}

bool DataBase::execute_with_return(const std::string & cmd, Result & result) {
    // lock
    std::lock_guard<std::mutex> lock(mutex_);
    // values
    sqlite3_stmt* pStmt;
    std::map<std::string, Data::ptr> values;

    // sql prepare
    int ret = sqlite3_prepare_v2(db_, cmd.c_str(), -1, &pStmt, nullptr);
    if ( ret != SQLITE_OK) {
        EXPERIENCE_FMT_ERR("sql statemenet with return exec failed, %s failed", cmd.c_str());
        return false;
    }

    // get column count
    int count = sqlite3_column_count(pStmt);
    // read message from data
    do {
        // step next when sqlite busy, can re run
        ret = sqlite3_step(pStmt);
        // check row
        // TODO here should optimize code
        if (ret == SQLITE_ROW) {
            std::map<std::string, Data::ptr> values;
            // run to get index
            int typ;
            for (int index = 0; index < count; index++) {
                typ = sqlite3_column_type(pStmt, index);
                // store to string
                char* buf;
                size_t size;
                Data::ptr ptr;
                // column name
                const std::string name = sqlite3_column_name(pStmt, index);
                // save key and type
                ptr->typ = typ;
                // insert
                values.insert(std::make_pair(name, ptr));
                switch (typ) {
                // int type
                case SQLITE_INTEGER:
                    {
                        // store to type
                        int value = sqlite3_column_int(pStmt, index);
                        buf = reinterpret_cast<char*>(&value);
                        ptr->data = std::string(buf, sizeof(int));
                        break;
                    }
                case SQLITE_FLOAT:
                    {
                        // store to type
                        double value = sqlite3_column_double(pStmt, index);
                        buf = reinterpret_cast<char*>(&value);
                        ptr->data = std::string(buf, sizeof(double));
                        break;
                    }
                case SQLITE_TEXT:
                    {
                        auto value = sqlite3_column_text(pStmt, index);
                        buf = reinterpret_cast<char*>(&value);
                        ptr->data = std::string(buf);
                        break;
                    }
                // dont store binary 
                case SQLITE_BLOB:
                default:
                    EXPERIENCE_ASSERT(false);
                }
            }
            // store to vec
            result.emplace_back(values);
        }    
    // check if read done or read error
    } while (ret != SQLITE_DONE || ret == SQLITE_ERROR);

    // free
    sqlite3_free(pStmt);

    if (ret == SQLITE_ERROR) {
        return false;
    }
    EXPERIENCE_FMT_DEBUG("sql statement with return exec successfully, cmd: %s", cmd.c_str());
    return true;
}

// execute command
bool DataBase::execute_no_return(const std::string & cmd) {
    // lock
    std::lock_guard<std::mutex> lock(mutex_);
    char* errmsg = nullptr;
    // execute sql statement
    if (sqlite3_exec(db_, cmd.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
        EXPERIENCE_FMT_ERR("sql statemenet no return exec failed, cmd: %s,err: %s", cmd.c_str(), errmsg);
        sqlite3_free(errmsg);
        return false;
    }
    sqlite3_free(errmsg);
    EXPERIENCE_FMT_DEBUG("sql statement exec successfully, cmd: %s", cmd.c_str());
    return true;
}

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


DBModule::~DBModule() {
    
}

// connect to database
void DBModule::connect(const std::string &url) {
    // must nullptr
    EXPERIENCE_ASSERT(db_ == nullptr);
    db_.reset(new DataBase(url));
    db_->open();
    // create table
    create_table(database_table);
    // save table name
    table_ = url;
}

// disconnect from database
void DBModule::disconnect() {
    // must 
    EXPERIENCE_ASSERT(db_ != nullptr);
    db_->close();
}

// create table
void DBModule::create_table(const std::string & table) {
    // table type
    const std::string typ = R"(
        Type INTEGER,
        Data TEXT,
        Nano TIMESTAMP,
    )";
    db_->create_table(table, typ);
}

const std::string DBModule::get_table() {
    EXPERIENCE_ASSERT(table_ != "");
    return table_;
}

// collect message to queue
void DBModule::collect(QueueInterface::ptr que) {
    std::vector<std::string> vec;
    // read data from database
    if (!read(vec)) {
        EXPERIENCE_FMT_ERR("read data from %s failed", table_.c_str());
        return;
    }
    // create req
    ReqMessage::ptr req(new ReqMessage());
    req->tid = TidTyp::GeneralTid;
    req->encode = nullptr;
    req->call_back = std::bind(&DBModule::handler, this, std::placeholders::_1);
    req->vec = std::move(vec);
    // push to queue
    que->push(req);
}

// write data to database
void DBModule::write(QueueInterface::ptr que) {
    while (true) {
        // pop req
        auto req = que->pop();
        std::map<std::string, std::string> values;
        values.insert(std::make_pair("Type", std::to_string(int(req->tid))));
        // current time
        auto duration = std::chrono::system_clock::now().time_since_epoch();
        auto second = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        values.insert(std::make_pair("Nano", std::to_string(second)));
        for (auto iter : req->vec) {
            values.insert(std::make_pair("Data", iter));
            db_->insert(get_table(), values);
        }
        // data has been written
        EXPERIENCE_FMT_DEBUG("data write end, data: %s", req->debug().c_str());
    }
}

// read data
bool DBModule::read(std::vector<std::string> & vec) {
    // clear vector
    const std::string key = "Data";
    vec.clear();
    DataBase::Result result;
    if (!db_->read(get_table(), key, std::make_pair("", ""), result))
        return false;

    // parse result 
    for (auto iter : result) {
        // try to find
        auto finder = iter.find(key);
        // check if already in map end
        if (finder == iter.end()) 
            continue;
        // parse 
        auto data = finder->second;
        EXPERIENCE_ASSERT(data->typ == SQLITE_TEXT);
        // text can direct convert to string
        // dont need to parse here
        vec.emplace_back(data->data);
    }
    return true;
}


}