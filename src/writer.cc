#include "writer.h"
#include "define.h"
#include "define.pb.h"
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
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>

#include <sqlite3.h>
#include <cpr/api.h>
#include <cpr/body.h>
#include <cpr/cprtypes.h>
#include <cpr/response.h>
#include <cpr/timeout.h>
#include <boost/algorithm/string/join.hpp>
#include <google/protobuf/stubs/status.h>
#include <google/protobuf/util/json_util.h>

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
        EXPERIENCE_FMT_ERR("database open failed, err: %s", sqlite3_errmsg(db_));
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
    std::string cmd = "CREATE TABLE IF NOT EXISTS " + name + "(" + typ + ")";
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
    std::string db_key = boost::algorithm::join(keys, ",");
    std::string db_value = boost::algorithm::join(values, ",");
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
        Nano TIMESTAMP
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
            std::string encode_data = CryptorUtils::base64_encode(iter);
            values.insert(std::make_pair("Data", encode_data));
            db_->insert(get_table(), values);
        }
        // data has been written
        EXPERIENCE_FMT_DEBUG("data write end, data: %s", boost::algorithm::join(req->vec, ",").c_str());
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
        std::string decode_data = CryptorUtils::base64_decode(data->data);
        vec.emplace_back(decode_data);
    }
    return true;
}

const std::string DBModule::get_remote()  {
    return database_fle;
}

void DBModule::handler(ReqResult::ptr result) {
    // only saved result dont need to delete
    if (result->code != ReqResultCode::WriteResultSavedFailed) {
        EXPERIENCE_DEBUG("database post failed, but dont need to delete data");
        return;
    }
    // remove 
    if (!db_->remove(get_table(), std::make_pair("Data", result->origin))) {
        EXPERIENCE_FMT_WARN("delete data from database failed, data: %s", result->origin.c_str());
        return;
    }
    EXPERIENCE_FMT_DEBUG("delete data from database success, data: %s", result->origin.c_str());
}

UrlValues::UrlValues() {
    values_.clear();
}

void UrlValues::add(const std::string &key, const std::string &value) {
    values_.insert(std::make_pair(key, value));
}

void UrlValues::del(const std::string &key) {
    values_.erase(key);
}

// encode message
const std::string UrlValues::encode() {
    // check size
    switch (values_.size()) {
    case 0:
        return "";
    case 1:
        values_.cend();
        return values_.cbegin()->first + "=" + values_.cbegin()->second;
    }
    // add first elem
    std::string result;
    result.append(values_.cbegin()->first + "=" + values_.cbegin()->second);
    auto iter = values_.cbegin()++;
    for (; iter!= values_.cend(); iter++) {
        result.append("&");
        result.append(iter->first + "=" + iter->second);
    }
    return result;
}

// write to web
void WebWriter::write(QueueInterface::ptr que) {
    while (true) {
        // pop message
        auto req = que->pop();
        // send data
        auto result = send(req);
        // call handle
        if (req->call_back)
            req->call_back(result);
    }
}

// send data to web
ReqResult::ptr WebWriter::send(ReqMessage::ptr req) {
    // post data
    define::PostSimpleData data;
    // time
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto second = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    data.set_rt(second);
    // time zone
    data.set_zone(SystemInfo::get_zone());
    // unid
    data.set_unid(SystemInfo::get_unid());
    // add string 
    for (auto iter : req->vec) {
        data.add_data(iter);
    }
    // request result 
    ReqResult::ptr result;
    // convert to json
    namespace util = google::protobuf::util;
    std::string msg;
    if (!util::MessageToJsonString(data, &msg).ok()) {
        EXPERIENCE_FMT_ERR("message cant convert to json: msg:%s", data.DebugString().c_str());
        // unsaved result
        result->code = ReqResultCode::WriteResultUnsavedFailed;
        return result;
    }
    // 
    auto encode_data = CryptorUtils::aes_encode("");
    UrlValues param;
    param.add("aid", SystemInfo::get_aid());
    param.add("key", encode_data->key);

    // random urls 
    std::vector<std::string> urls = urls_;
    std::random_shuffle(urls.begin(), urls.end());

    // send data
    cpr::Response resp;
    for (auto iter : urls) {
        // post url
        std::string full_url = iter + "/" + post_unification + "?" + param.encode();
        // post urls
        resp = cpr::Post(cpr::Url(full_url), cpr::Header{{"Content-Type", "application/json"}}, cpr::Body{encode_data->result}, cpr::Timeout{300});
        // check if post successfully
        if (resp.status_code != 200) {
            EXPERIENCE_FMT_DEBUG("post data to web not reachable, url: %s, data: %s, state code: %d", 
                iter.c_str(), msg.c_str(), resp.status_code);
            continue;
        }
        EXPERIENCE_FMT_DEBUG("post data to web successfully, url: %s, data: %s", iter.c_str(), msg.c_str());
        break;
    }  

    // check is post successfully
    if (resp.status_code != 200) {
        EXPERIENCE_ERR("post data failed, all urls not avaliable");
        // unsaved result
        result->code = ReqResultCode::WriteResultUnsavedFailed;
        return result;
    }
    define::ResponseRcv rcv;
    if(!util::JsonStringToMessage(resp.text,&rcv).ok()) {
        EXPERIENCE_FMT_ERR("unmarshal rcv msg failed, msg: %s", resp.text.c_str());
        // unsaved result
        result->code = ReqResultCode::WriteResultUnsavedFailed;
        return result;
    }
    
    // check if request is valid for server
    if (rcv.code() != 0) {
        EXPERIENCE_FMT_ERR("post data failed, data is invalid: code: %d", rcv.code());
        result->code = ReqResultCode::WriteResultUnsavedFailed;
        return result;
    }
    // check if key is the same, must be the same key
    if (rcv.data().key() != encode_data->key) {
        EXPERIENCE_FMT_WARN("send key %s is differ with rcv key %s", encode_data->key.c_str(), rcv.data().key().c_str());
    }
    // result
    result->code = ReqResultCode::WriteResultSuccess;
    CryptResult::ptr crypted_result;
    crypted_result->key = encode_data->key;
    crypted_result->result = rcv.data().data();
    // decode
    result->msg =  CryptorUtils::aes_decode(crypted_result);
    return result;
}

const std::string WebWriter::get_remote() {
    return "";
}

}