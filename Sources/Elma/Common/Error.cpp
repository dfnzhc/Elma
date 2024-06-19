/**
 * @File Error.cpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/17
 * @Brief This file is part of Elma.
 */

#include "./Error.hpp"

namespace elma {

void ThrowException(const std::source_location& loc, std::string_view msg)
{
    std::string fullMsg  = std::format("{}\n\n{}:{} ({})", msg, loc.file_name(), loc.line(), loc.function_name());
    fullMsg             += std::format("\n\nStacktrace:\n{}", std::stacktrace::current(1));

    throw elma::RuntimeError(fullMsg);
}

void ReportAssertion(const std::source_location& loc, std::string_view cond, std::string_view msg)
{
    // clang-format off
    std::string fullMsg = std::format("断言失败: {}\n{}{}\n{}:{} ({})",
                                      cond, msg, msg.empty() ? "" : "\n",
                                      loc.file_name(), loc.line(), loc.function_name());
    // clang-format on
    fullMsg += std::format("\n\nStacktrace:\n{}", std::stacktrace::current(1));

    throw AssertionError(fullMsg);
}
} // namespace elma