#include <string>
#include <iostream>
#pragma once

enum log_level {DEBUG = 0, INFO, WARNING, ERROR, LogLevelMax};

class Loggeable {
public:
    void mute()   {m_mute = true;};
    void unmute() {m_mute = false;};

    void filter(log_level min_lvl) {m_min_lvl = min_lvl;};
    void prefix(std::string prefix) {m_prefix = std::move(prefix);};

    template<typename... T>
    void log(log_level lvl, const char* fmt, T ... args) {
        if(lvl >= m_min_lvl && !m_mute)
        {
            *out_streams[lvl] << m_prefix;

             // don't store on the stack; no need to be reentrant and allows coroutines to have a tiny stack
            static char buff[2048];
            std::snprintf(&buff[0], 2048, fmt, args...); // No buffer overflow there, sir !

            *out_streams[lvl] << &buff[0];
        }
    };

    bool m_mute = false;
    log_level m_min_lvl = INFO;
    std::string m_prefix;
    std::ostream* out_streams[LogLevelMax] =
    {
        &std::clog, // DEBUG
        &std::cout, // INFO
        &std::cerr, // WARNING
        &std::cerr  // ERROR
    };
};

inline Loggeable global_logger;

template<typename... T>
void log(log_level lvl, const char* fmt, T ... args)
{
    global_logger.log(lvl, fmt, args...);
}

template<typename... T>
void info(const char* fmt, T ... args)
{
    global_logger.log(INFO, fmt, args...);
}

template<typename... T>
void warn(const char* fmt, T ... args)
{
    global_logger.log(WARNING, fmt, args...);
}

template<typename... T>
void error(const char* fmt, T ... args)
{
    global_logger.log(ERROR, fmt, args...);
}
