#include "writer.h"
#include "log.h"

#include <exception>
#include <mutex>
#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace experience {

DataBase::DataBase(const std::string & path) {
    // save and check
    path_ = path;
    EXPERIENCE_ASSERT(db_ == nullptr);
}

DataBase::~DataBase() {
    sqlite3_close_v2(db_);
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

// check if database is opened
bool DataBase::is_open() {
    return db_ != nullptr;
}

// create table
bool DataBase::create_table(const std::string & name, const std::string &typ) {
    // create table command
    std::string cmd = "CREATE TABLE IF NOT EXIST " + name + "(" + typ + ");";
    if (!execute(cmd)) {
        EXPERIENCE_FMT_ERR("create table %s failed", name.c_str());
        return false;
    }
    EXPERIENCE_FMT_INFO("create table %s successfully", name.c_str());
    return true;
}

// insert data to table
bool DataBase::insert(const std::string & table, std::map<std::string, std::string> && variant) {

}


// execute command
bool DataBase::execute(const std::string & cmd) {
    std::lock_guard<std::mutex> lock(mutex_);
    char *errmsg = nullptr;
    // execute sql statement
    if (sqlite3_exec(db_, cmd.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
        EXPERIENCE_FMT_ERR("sql statemenet exec failed, cmd: %s,err: %s", cmd.c_str(), errmsg);
        sqlite3_free(errmsg);
        return false;
    }
    sqlite3_free(errmsg);
    EXPERIENCE_FMT_DEBUG("sql statement exec successfully, cmd: %s", cmd.c_str());
    return true;
}


DBModule::~DBModule() {
    
}


void DBModule::connect(const std::string &url) {

}

// create table
void DBModule::create_table(const std::string & table) {
    const std::string typ = R"(
        Type INTEGER,
        Data TEXT,
        Nano DATETIME,
    )";
    std::string sql = "CREATE TABLE IF NOT EXIST " + table + "(" + typ + ");";
    EXPERIENCE_FMT_DEBUG("execute create table command %s", sql.c_str());


}


}