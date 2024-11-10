#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <cstdint>
#include <ios>

namespace zipper {
enum class log_level {
    debug = 0,
    error = 1,
    panic = 2,
    info  = 3,
    warning = 4
};
static const char* _log_level_string[] = {
    "DEBUG",
    "ERROR",
    "PANIC",
    "INFO",
    "WARNING"
};

class logger {
    std::ostream& m_stream;
    log_level level;
public:

    static logger& get_logger();
    

    logger(std::ostream& stream, log_level lvl) : m_stream(stream), level(lvl) {}
    void log(log_level lvl) {
        m_stream << "[" << _log_level_string[static_cast<uint32_t>(lvl)] << "] ";
    }
    
    template<typename T>
    void log(T value) {
        m_stream << value << std::endl;
    }

    template<typename T, typename... Args>
    void log(T value, Args... args){
        m_stream << value << " ";
        log(args...);
    }

    template<typename... Args>
    void log(log_level lvl, Args... args) {
        log(lvl);
        log(args...);
    }
};
}

#endif
