#include "EventLogger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

EventLogger &EventLogger::instance()
{
    static EventLogger inst;
    return inst;
}

void EventLogger::init(const std::string &filename)
{
    if (m_initialized) return;

    const size_t maxSize  = 10 * 1024 * 1024;
    const size_t maxFiles = 5;

    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        filename, maxSize, maxFiles);
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    auto logger = std::make_shared<spdlog::logger>(
        "netguard",
        spdlog::sinks_init_list{fileSink, consoleSink});

    logger->set_level(spdlog::level::info);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::set_default_logger(logger);

    m_initialized = true;
}

void EventLogger::info (const std::string &msg) { spdlog::info(msg);  }
void EventLogger::warn (const std::string &msg) { spdlog::warn(msg);  }
void EventLogger::error(const std::string &msg) { spdlog::error(msg); }
void EventLogger::debug(const std::string &msg) { spdlog::debug(msg); }
