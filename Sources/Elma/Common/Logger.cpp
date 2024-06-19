/**
 * @File Logger.cpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/5/13
 * @Brief 
 */

#include "Logger.hpp"
#include "./Defines.hpp"
#include "./Terminal.hpp"

#include <iostream>
#include <ranges>

using namespace elma;

namespace {
constexpr inline const char* logLevelString(Logger::Level level)
{
    switch (level) {
    case Logger::Level::Fatal   : return "Fatal";
    case Logger::Level::Error   : return "Error";
    case Logger::Level::Warning : return "Warning";
    case Logger::Level::Info    : return "Info";
    }
    // TODO: not implementation
}

} // namespace

void Logger::log(Level level, std::string_view msg)
{
    auto lock = std::lock_guard(_mutex);

    if (level > _level)
        return;

    const auto s = std::format("[{:>7}]: {}\n", logLevelString(level), msg);
    {
        auto& os = level > Logger::Level::Error ? std::cout : std::cerr;

        if (level == Logger::Level::Info)
            os << tc::reset << s;
        else if (level == Logger::Level::Warning)
            os << tc::coral << s;
        else if (level == Logger::Level::Error)
            os << tc::magenta << s;
        else if (level == Logger::Level::Fatal)
            os << tc::red << s;

        os << tc::reset << std::flush;
    }

    if (!_subscribers.empty()) {
        for (const auto& notify : _subscribers | std::views::values) {
            notify(s);
        }
    }
}

void Logger::setLevel(Level level)
{
    auto lock = std::lock_guard(_mutex);
    _level    = level;
}

Logger::Level Logger::level()
{
    auto lock = std::lock_guard(_mutex);
    return _level;
}

void Logger::subscribe(std::string_view name, LogNotifyType&& notify)
{
    auto lock = std::lock_guard(_mutex);
    _subscribers.emplace(name, std::move(notify));
}

void Logger::unsubscribe(std::string_view name)
{
    auto lock = std::lock_guard(_mutex);
    _subscribers.extract(name);
}

void detail::LogWithSourceLocation(Logger::Level level, std::source_location sl, std::string_view msg)
{
    Logger::inst().log(
        level, std::format("{}: '{}' {}({}:{})", msg, sl.function_name(), sl.file_name(), sl.line(), sl.column()));
}
