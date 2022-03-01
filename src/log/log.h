/*
 * Copyright (C) 2014 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     ArisAachen <chenyunxiong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __EXPERIENCE_SRC_LOG_H__
#define __EXPERIENCE_SRC_LOG_H__

#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>
#include <memory>
#include <chrono>
#include <vector>
#include <map>

namespace experience {

// LogLevel use to convert level
class LogLevel {
public:
    enum class Level { Fatal, Err, Warn, Info, Debug };
    /**
     * @brief transform class level to string
     * @param[in] level class type log level
     */
    static const std::string level_to_string(Level level);

    /**
     * @brief transform string to class level
     * @param[in] level string type level
     */
    static const Level string_to_level(const std::string & level);
};

// LogEvent indicate current event
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    typedef std::chrono::time_point<std::chrono::system_clock> Clock;
    /**
     * @brief Construct a new Log Event object
     * @param[in] clock log time
     * @param[in] user user host name
     * @param[in] name program name
     * @param[in] pid process id
     * @param[in] file log file name
     * @param[in] func log function name
     * @param[in] line log line
     * @param[in] message log message info
     */
    LogEvent(Clock clock, const std::string & user, const std::string & name, uint64_t pid, const std::string & file, 
        const std::string & func, uint64_t line, const std::string & message):
        time_(clock), user_(user), name_(name), pid_(pid), file_(file), func_(func), line_(line), message_(message)
        {}

    /**
     * @brief get log file name
     */
    const std::string get_file() { return file_; }

    /**
     * @brief get log func name
     */
    const std::string get_func() { return func_; }

    /**
     * @brief get log line
     */
    uint64_t get_line() { return line_; }

    /**
     * @brief get log process name
     */
    const std::string get_name() { return name_; }

    /**
     * @brief get log process id
     */
    uint64_t get_pid() { return pid_; }    

    /**
     * @brief get log time
     */
    const Clock get_clock() { return time_; }

    /**
     * @brief get log process name
     */
    const std::string get_user() { return user_; }

    /**
     * @brief get log message
     */
    const std::string get_message() { return message_; }

private:
    /// code file
    std::string file_ {""};
    /// code func
    std::string func_ {""};
    /// code line
    uint64_t line_ {0};
    
    /// program name
    std::string name_ {""};
    /// process id 
    uint64_t pid_ {0};

    /// log time
    Clock time_;
    /// user host name
    std::string user_ {""};
    /// log message
    std::string message_ {""};
};

// LogFormater use to format message
class LogFormater {
public:
    typedef std::shared_ptr<LogFormater> ptr;
    
    /**
     * @brief log pattern format
     * @param[in] pattern use to format a log
     * @details 
     * %d time format
     * %T table
     * %u host name
     * %N process name
     * %p proccess id
     * %L log level
     * %f file name
     * %c func name
     * %l log line
     * %m log message
     * %n new line
     */
    LogFormater(const std::string pattern = "%d{%Y-%m-%d %H:%M:%S}%T%u%T%N[%p]:%T<%L>%T%f:%c:%l%T%m%n");

    /**
     * @brief Destroy the virtual Log Formater object
     */
    virtual~LogFormater();

    /**
     * @brief Get the format object
     */
    const std::string get_format() { return log_pattern_; }

    /**
     * @brief init log format
     */
    void init();

    /**
     * @brief log 
     * @param[in] level log level
     * @param[in] event log event
     */
    const std::string format(LogLevel::Level level, LogEvent::ptr event);

    /**
     * @brief 
     * @param os ostream
     * @param level log level
     * @param event log event
     */
    std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event);    

public:
    // FormaterItem format item
    class FormaterItem {
    public:
        typedef std::shared_ptr<FormaterItem> ptr;
        /**
         * @brief Construct a new Formater Item object
         * @param[in] extend has extend info to format item
         */
        FormaterItem(const std::string & extend): extend_(extend) {}

        /**
         * @brief 
         * @param[in] level log level
         * @param[in] event log event
         */
        virtual const std::string format(LogLevel::Level level, LogEvent::ptr event) {
            std::stringstream ss;
            format(ss, level, event);
            return ss.str();
        };

        /**
         * @brief 
         * @param[in] os in and out ostream
         * @param[in] level log level
         * @param[in] event log event
         */
        virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) = 0;

    protected:
        /// carry extends message
        std::string extend_;
    };

private:
    /// log format pattern
    std::string log_pattern_ {""};
    /// log format items
    std::vector<FormaterItem::ptr> items_;
};

// LogAppender appender 
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;

    /**
     * @brief must init
     */
    virtual void init() = 0;

    /**
     * @brief 
     * @param[in] level log level
     * @param[in] event
     */
    virtual void format(LogLevel::Level level, LogEvent::ptr event) = 0;
    
    /**
     * @brief Set the level object
     * @param[in] level set log level 
     */
    virtual void set_level(LogLevel::Level level) = 0;

protected:
    /// log formater 
    LogFormater::ptr formater_;
    /// log level
    LogLevel::Level level_ { LogLevel::Level::Info };
};


class Logger {
public:
    /**
     * @brief Construct a new Logger object
     */
    Logger();

    /**
     * @brief use default log appender
     */
    void use_default();

    /**
     * @brief add appender to log map
     * @param name appender name
     * @param appender appender obj
     */
    void add_appender(const std::string & name, LogAppender::ptr appender);

    /**
     * @brief delete appender
     * @param[in] appender delete appender name
     */
    void delete_appender(const std::string & name);

    /**
     * @brief log
     * @param[in] level log level
     * @param[in] event log event
     */
    void log(LogLevel::Level level, LogEvent::ptr event);

    /**
     * @brief debug log
     * @param[in] event log event
     */
    void debug(LogEvent::ptr event);

    /**
     * @brief info log
     * @param[in] event log event
     */
    void info(LogEvent::ptr event);

    /**
     * @brief warn log
     * @param[in] event log event
     */
    void warn(LogEvent::ptr event);

    /**
     * @brief error log
     * @param[in] event log event
     */
    void error(LogEvent::ptr event);

    /**
     * @brief fatal log
     * @param[in] event log event
     */
    void fatal(LogEvent::ptr event);

private:
    /// log appenders
    std::map<std::string, LogAppender::ptr> appenders_;
};


}

#endif