#pragma once
#include <string>

class EventLogger
{
public:
    static EventLogger &instance();

    void init(const std::string &filename);

    void info (const std::string &msg);
    void warn (const std::string &msg);
    void error(const std::string &msg);
    void debug(const std::string &msg);

private:
    EventLogger() = default;
    bool m_initialized = false;
};
