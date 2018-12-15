#include <string>

#pragma once

enum log_level {INFO = 0, WARNING = 1, ERROR = 2};

struct log_message {std::string str, log_level lvl;};

class Loggeable {
public:
       void mute()   {m_mute = true;};
       void unmute() {m_mute = false;};

       void filter(log_level min_lvl) {m_min_lvl = min_lvl;};
       void prefix(std::string prefix) {m_prefix = prefix;};

private:
       void log(log_message a) {if(a.lvl >= m_min_lvl && !m_mute){std::clog << m_prefix << a.str << '\n';}};

       bool m_mute = false;
       log_level m_min_lvl = INFO;
       std::string m_prefix;
};
