/**
 * @File Error.hpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/15
 * @Brief This file is part of Elma.
 */

#pragma once

#include <format>
#include <exception>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <source_location>
#include <stacktrace>
#include "./Enums.hpp"
#include "./Logger.hpp"

namespace elma {

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable:4'275) // allow dllexport on classes dervied from STL
#endif

class Exception : public std::exception
{
public:
    Exception() noexcept = default;

    explicit Exception(std::string_view what) : _pWhat(std::make_shared<std::string>(what)) { }

    Exception(const Exception& other) noexcept : exception(other) { _pWhat = other._pWhat; }

    ~Exception() override = default;

    Exception& operator=(const Exception&) = delete;

    const char* what() const noexcept override { return _pWhat ? _pWhat->c_str() : ""; }

protected:
    std::shared_ptr<std::string> _pWhat;
};

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

class RuntimeError : public Exception
{
public:
    RuntimeError() noexcept = default;

    explicit RuntimeError(std::string_view what) : Exception(what) { }

    RuntimeError(const RuntimeError& other) noexcept : Exception(other) { _pWhat = other._pWhat; }

    ~RuntimeError() override = default;
};

class AssertionError : public Exception
{
public:
    AssertionError() noexcept = default;

    explicit AssertionError(std::string_view what) : Exception(what) { }

    AssertionError(const AssertionError& other) noexcept : Exception(other) { _pWhat = other._pWhat; }

    ~AssertionError() override = default;
};

[[noreturn]] void ThrowException(const std::source_location& loc, std::string_view msg);

namespace internal {

[[noreturn]] inline void ThrowException(const std::source_location& loc, std::string_view msg)
{
    ::elma::ThrowException(loc, msg);
}

template<typename... Args>
[[noreturn]] inline void
ThrowException(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
{
    ::elma::ThrowException(loc, std::format(fmt, std::forward<Args>(args)...));
}
} // namespace internal
} // namespace elma

#define ELMA_THROW(...) ::elma::internal::ThrowException(std::source_location::current(), __VA_ARGS__)

#define ELMA_CHECK(cond, ...)                                                                                          \
    do {                                                                                                               \
        if (!(cond))                                                                                                   \
            ELMA_THROW(__VA_ARGS__);                                                                                   \
    } while (0)

#define ELMA_UNIMPLEMENTED() ELMA_THROW("还未实现的部分")

#define ELMA_UNREACHABLE() ELMA_THROW("You shall not PASS!!!")

namespace elma {

[[noreturn]] void ReportAssertion(const std::source_location& loc, std::string_view cond, std::string_view msg = {});

namespace internal {

[[noreturn]] inline void ReportAssertion(const std::source_location& loc, std::string_view cond)
{
    ::elma::ReportAssertion(loc, cond);
}

[[noreturn]] inline void ReportAssertion(const std::source_location& loc, std::string_view cond, std::string_view msg)
{
    ::elma::ReportAssertion(loc, cond, msg);
}

template<typename... Args>
[[noreturn]] inline void
ReportAssertion(const std::source_location& loc, std::string_view cond, std::format_string<Args...> fmt, Args&&... args)
{
    ::elma::ReportAssertion(loc, cond, std::format(fmt, std::forward<Args>(args)...));
}
} // namespace internal
} // namespace elma

#ifdef ELMA_ENABLE_ASSERTS

#  define ELMA_ASSERT(cond, ...)                                                                                       \
      do {                                                                                                             \
          if (!(cond)) {                                                                                               \
              ::elma::internal::ReportAssertion(std::source_location::current(), #cond, __VA_ARGS__);                  \
          }                                                                                                            \
      } while (0)

#  define ELMA_ASSERT_OP(a, b, OP)                                                                                     \
      do {                                                                                                             \
          if (!(a OP b)) {                                                                                             \
              ::elma::internal::ReportAssertion(std::source_location::current(),                                       \
                                                std::format("{} {} {} ({} {} {})", #a, #OP, #b, a, #OP, b));           \
          }                                                                                                            \
      } while (0)

#  define ELMA_ASSERT_EQ(a, b) ELMA_ASSERT_OP(a, b, ==)
#  define ELMA_ASSERT_NE(a, b) ELMA_ASSERT_OP(a, b, !=)
#  define ELMA_ASSERT_GE(a, b) ELMA_ASSERT_OP(a, b, >=)
#  define ELMA_ASSERT_GT(a, b) ELMA_ASSERT_OP(a, b, >)
#  define ELMA_ASSERT_LE(a, b) ELMA_ASSERT_OP(a, b, <=)
#  define ELMA_ASSERT_LT(a, b) ELMA_ASSERT_OP(a, b, <)

#else // ELMA_ENABLE_ASSERTS

#  define ELMA_ASSERT(cond, ...)                                                                                       \
      do {                                                                                                             \
          {                                                                                                            \
          }                                                                                                            \
      } while (0)
#  define ELMA_ASSERT_OP(a, b, OP)                                                                                     \
      do {                                                                                                             \
          {                                                                                                            \
          }                                                                                                            \
      } while (0)
#  define ELMA_ASSERT_EQ(a, b) ELMA_ASSERT_OP(a, b, ==)
#  define ELMA_ASSERT_NE(a, b) ELMA_ASSERT_OP(a, b, !=)
#  define ELMA_ASSERT_GE(a, b) ELMA_ASSERT_OP(a, b, >=)
#  define ELMA_ASSERT_GT(a, b) ELMA_ASSERT_OP(a, b, >)
#  define ELMA_ASSERT_LE(a, b) ELMA_ASSERT_OP(a, b, <=)
#  define ELMA_ASSERT_LT(a, b) ELMA_ASSERT_OP(a, b, <)

#endif // ELMA_ENABLE_ASSERTS

namespace elma {

template<typename CallbackT, typename ResultT = int>
inline int CatchAndReportAllExceptions(CallbackT callback, ResultT errorResult = 1)
{
    ResultT result = errorResult;
    try {
        result = callback();
    } catch (const AssertionError& e) {
        LogError(std::string("断言错误:\n\n") + e.what());
    } catch (const std::exception& e) {
        LogFatal(std::string("发生异常:\n\n") + e.what());
    } catch (...) {
        LogFatal("未知异常发生");
    }
    return result;
}

} // namespace elma