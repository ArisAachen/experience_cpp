#include "log.h"

#include <array>
#include <locale>
#include <ctime>
#include <iomanip>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <map>
#include <functional>

namespace experience {

// level_to_string use to convert class type level to string
const std::string LogLevel::level_to_string(Level level) {
    switch (level) {
// type to string
#define TTS(name)  \
    case Level::name: \
        return #name;
    // return string level
    TTS(Debug);
    TTS(Info);
    TTS(Warn);
    TTS(Err);
    TTS(Fatal);
    default:
        break;
    }
    // if all level is not match, level is default info 
    return "Info";
}

// string_to_level use to convert string to class type
const LogLevel::Level LogLevel::string_to_level(const std::string &level) {
// string to type
#define STT(name) \
    if (level == #name) \
        return Level::name;
    // return class level
    STT(Debug);
    STT(Info);
    STT(Warn);
    STT(Err);
    STT(Fatal);
    // if level is not match, level is default info
    return Level::Info;
}

LogFormater::LogFormater(const std::string pattern):
log_pattern_(pattern) {
    items_.clear();
}

LogFormater::~LogFormater() {
    items_.clear();
}


// StringFormatItem use to format simple message
class StringFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << extend_;
    }
};

// DateTimeFormatItem %d{%Y-%m-%d %H:%M:%S} 
class DateTimeFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        // get time clock
        auto seconds = std::chrono::system_clock::to_time_t(event->get_clock());
        return os << std::put_time(std::localtime(&seconds), extend_.c_str());
    }
};

// TableFormatItem %T
class TableFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << "\t";
    }
};

// UserFormatItem %u
class UserFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_user();
    }
};

// ProcNameFormatItem %N
class ProcNameFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_name();
    }
};

// ProcIdFormatItem %p
class ProcIdFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_pid();
    }
};

// LogLevelFormatItem %L
class LogLevelFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << LogLevel::level_to_string(level);
    }
};

// FineNameFormatItem %f
class FineNameFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_file();
    }
}; 

// FuncNameFormatItem %c
class FuncNameFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_func();
    }
}; 

// FuncLineFormatItem %l
class FuncLineFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_line();
    }
};

// FuncLineFormatItem %l
class MessageFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_message();
    }
};

// LineFormatItem %n
class NewLineFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << "\n";
    }
};

// init format to vec
void LogFormater::init() {
    // type extend success
    std::vector<std::tuple<std::string, std::string, bool>> vec;

    // parse format
    for (int index = 0; index < log_pattern_.size(); index++) {
        // should ignore % character
        if (log_pattern_[index] == '%')
            continue;
        
        // parse time format here
        // %d{%Y-%m-%d %H:%M:%S}
        if (log_pattern_[index] == 'd') {
            int n = index + 1;
            // check if time format is legal
            if (log_pattern_[n] != '{') {
                vec.emplace_back("d", "%Y-%m-%d %H:%M:%S", true);
                break;
            }
            // search time format end
            while (n < log_pattern_.size()) {
                // add 
                if (log_pattern_[n] != '}') {
                    n++;
                    continue;
                }
                // add date time item 
                vec.emplace_back("d", std::string(log_pattern_[index+2], n-index-2), true);
                index = n + 1;
                break;
            }
        }
        // add other item
        vec.emplace_back(std::string(log_pattern_[index], 1), "", true);
    }

    // format items
    static const std::map<std::string, std::function<FormaterItem::ptr(const std::string &)>> items = {
#define MAKE_MAP(str, C) \
        {#str, [](const std::string & name) { return FormaterItem::ptr(new C(name)); }}
        
        MAKE_MAP(d, DateTimeFormatItem), 
        MAKE_MAP(T, TableFormatItem),
        MAKE_MAP(u, UserFormatItem),
        MAKE_MAP(N, ProcNameFormatItem),
        MAKE_MAP(p, ProcIdFormatItem),
        MAKE_MAP(L, LogLevelFormatItem),
        MAKE_MAP(f, FineNameFormatItem),
        MAKE_MAP(c, FuncNameFormatItem),
        MAKE_MAP(l, FuncLineFormatItem),
        MAKE_MAP(m, MessageFormatItem),
        MAKE_MAP(n, NewLineFormatItem)
    };
    // search format item 
    for (auto iter : vec) {
        auto finder = items.find(std::get<0>(iter));
        // cannot find format, output origin style
        if (finder == items.end()) {
            items_.emplace_back(StringFormatItem(std::get<1>(iter)));
        } else {
            // append 
            items_.emplace_back(finder->second(std::get<1>(iter)));
        }
    }
}



}