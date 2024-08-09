#pragma once

#include "Elma.hpp"

#include <chrono>
#include <ctime>

namespace elma {
/// For measuring how long an operation takes.
/// C++ chrono unfortunately makes the whole type system very complicated.
struct Timer
{
    std::chrono::time_point<std::chrono::system_clock> last;
};

inline Real tick(Timer& timer)
{
    const auto now     = std::chrono::system_clock::now();
    const auto elapsed = now - timer.last;
    Real ret           = elapsed.count();
    timer.last         = now;
    return ret;
}

} // namespace elma