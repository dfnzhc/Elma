/**
 * @File Terminal.hpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/5/18
 * @Brief 
 */

#pragma once

#include "./Defines.hpp"

#include <iostream>

#if defined(ELMA_IN_WINDOWS)
#  define TERMINAL_IN_WINDOWS
#elif defined(ELMA_IN_LINUX) || defined(ELMA_IN_MAC)
#  define TERMINAL_IN_POSIX
#endif

// These headers provide isatty()/fileno() functions, which are used for
// testing whether a standard stream refers to the terminal.
#if defined(TERMINAL_IN_POSIX)
#  include <unistd.h>
#  define ISATTY isatty
#  define FILENO fileno
#elif defined(TERMINAL_IN_WINDOWS)
#  include <io.h>
#  include <Windows.h>
#  define ISATTY _isatty
#  define FILENO _fileno
#endif

namespace elma {

namespace dbg {

inline bool isTTY([[maybe_unused]] const std::ostream& stream)
{
#ifdef NOVA_FORCE_USE_ANIS
    return true;
#else
    if (&stream == &std::cout && ISATTY(FILENO(stdout)))
        return true;
    if ((&stream == &std::cerr || &stream == &std::clog) && ISATTY(FILENO(stderr)))
        return true;

    return false;
#endif
}

} // namespace dbg

namespace tc {

constexpr inline bool isColorized(const std::ostream& stream)
{
    return dbg::isTTY(stream);
}

// -------------------------
// styles
// -------------------------

constexpr inline std::ostream& reset(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[00m";
    }
    return stream;
}

constexpr inline std::ostream& bold(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[1m";
    }
    return stream;
}

constexpr inline std::ostream& dark(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[2m";
    }
    return stream;
}

constexpr inline std::ostream& italic(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[3m";
    }
    return stream;
}

constexpr inline std::ostream& underline(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[4m";
    }
    return stream;
}

constexpr inline std::ostream& blink(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[5m";
    }
    return stream;
}

constexpr inline std::ostream& reverse(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[7m";
    }
    return stream;
}

constexpr inline std::ostream& concealed(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[8m";
    }
    return stream;
}

constexpr inline std::ostream& crossed(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[9m";
    }
    return stream;
}

// -------------------------
// colors
// -------------------------

template<uint8_t code> constexpr inline std::ostream& color(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[38;5;" << +code << "m";
    }
    return stream;
}

template<uint8_t r, uint8_t g, uint8_t b> constexpr inline std::ostream& color(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[38;2;" << +r << ";" << +g << ";" << +b << "m";
    }
    return stream;
}

template<uint8_t code> constexpr inline std::ostream& bg_color(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[48;5;" << +code << "m";
    }
    return stream;
}

template<uint8_t r, uint8_t g, uint8_t b> constexpr inline std::ostream& bg_color(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[48;2;" << +r << ";" << +g << ";" << +b << "m";
    }
    return stream;
}

constexpr inline std::ostream& grey(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[30m";
    }
    return stream;
}

constexpr inline std::ostream& red(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[31m";
    }
    return stream;
}

constexpr inline std::ostream& green(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[32m";
    }
    return stream;
}

constexpr inline std::ostream& yellow(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[33m";
    }
    return stream;
}

constexpr inline std::ostream& blue(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[34m";
    }
    return stream;
}

constexpr inline std::ostream& magenta(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[35m";
    }
    return stream;
}

constexpr inline std::ostream& cyan(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[36m";
    }
    return stream;
}

constexpr inline std::ostream& white(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[37m";
    }
    return stream;
}

constexpr inline std::ostream& bright_grey(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[90m";
    }
    return stream;
}

constexpr inline std::ostream& bright_red(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[91m";
    }
    return stream;
}

constexpr inline std::ostream& bright_green(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[92m";
    }
    return stream;
}

constexpr inline std::ostream& bright_yellow(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[93m";
    }
    return stream;
}

constexpr inline std::ostream& bright_blue(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[94m";
    }
    return stream;
}

constexpr inline std::ostream& bright_magenta(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[95m";
    }
    return stream;
}

constexpr inline std::ostream& bright_cyan(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[96m";
    }
    return stream;
}

constexpr inline std::ostream& bright_white(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[97m";
    }
    return stream;
}

constexpr inline std::ostream& bg_grey(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[40m";
    }
    return stream;
}

constexpr inline std::ostream& bg_red(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[41m";
    }
    return stream;
}

constexpr inline std::ostream& bg_green(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[42m";
    }
    return stream;
}

constexpr inline std::ostream& bg_yellow(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[43m";
    }
    return stream;
}

constexpr inline std::ostream& bg_blue(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[44m";
    }
    return stream;
}

constexpr inline std::ostream& bg_magenta(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[45m";
    }
    return stream;
}

constexpr inline std::ostream& bg_cyan(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[46m";
    }
    return stream;
}

constexpr inline std::ostream& bg_white(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[47m";
    }
    return stream;
}

constexpr inline std::ostream& bg_bright_grey(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[100m";
    }
    return stream;
}

constexpr inline std::ostream& bg_bright_red(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[101m";
    }
    return stream;
}

constexpr inline std::ostream& bg_bright_green(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[102m";
    }
    return stream;
}

constexpr inline std::ostream& bg_bright_yellow(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[103m";
    }
    return stream;
}

constexpr inline std::ostream& bg_bright_blue(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[104m";
    }
    return stream;
}

constexpr inline std::ostream& bg_bright_magenta(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[105m";
    }
    return stream;
}

constexpr inline std::ostream& bg_bright_cyan(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[106m";
    }
    return stream;
}

constexpr inline std::ostream& bg_bright_white(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << "\033[107m";
    }
    return stream;
}

constexpr inline std::ostream& coral(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << color<255, 127, 80>;
    }
    return stream;
}

constexpr inline std::ostream& bg_coral(std::ostream& stream)
{
    if (isColorized(stream)) {
        stream << bg_color<255, 127, 80>;
    }
    return stream;
}

} // namespace tc

} // namespace elma

#undef TERMINAL_IN_POSIX
#undef TERMINAL_IN_WINDOWS
