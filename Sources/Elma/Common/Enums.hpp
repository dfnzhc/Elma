/**
 * @File Enums.hpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/15
 * @Brief 
 */

#pragma once

#define ELMA_ENUM_BIT_OPERATORS(e_)                                                                                    \
    inline e_ operator&(e_ a, e_ b)                                                                                    \
    {                                                                                                                  \
        return static_cast<e_>(static_cast<int>(a) & static_cast<int>(b));                                             \
    }                                                                                                                  \
    inline e_ operator|(e_ a, e_ b)                                                                                    \
    {                                                                                                                  \
        return static_cast<e_>(static_cast<int>(a) | static_cast<int>(b));                                             \
    }                                                                                                                  \
    inline e_& operator|=(e_& a, e_ b)                                                                                 \
    {                                                                                                                  \
        a = a | b;                                                                                                     \
        return a;                                                                                                      \
    };                                                                                                                 \
    inline e_& operator&=(e_& a, e_ b)                                                                                 \
    {                                                                                                                  \
        a = a & b;                                                                                                     \
        return a;                                                                                                      \
    };                                                                                                                 \
    inline e_ operator~(e_ a)                                                                                          \
    {                                                                                                                  \
        return static_cast<e_>(~static_cast<int>(a));                                                                  \
    }                                                                                                                  \
    inline bool isSet(e_ val, e_ flag)                                                                                 \
    {                                                                                                                  \
        return (val & flag) != static_cast<e_>(0);                                                                     \
    }                                                                                                                  \
    inline void FlipBit(e_& val, e_ flag)                                                                              \
    {                                                                                                                  \
        val = isSet(val, flag) ? (val & (~flag)) : (val | flag);                                                       \
    }

#include <concepts>
#include <type_traits>
#include "./Defines.hpp"

namespace elma {

template<typename T, typename E>
    requires std::is_enum_v<E> and std::is_nothrow_convertible_v<T, std::underlying_type_t<E>>
ELMA_FUNC constexpr T cast_to(E e) noexcept
{
    return static_cast<T>(e);
}

template<typename TE, typename E>
    requires std::is_enum_v<TE> and std::is_enum_v<E> and
             std::is_nothrow_convertible_v<std::underlying_type_t<TE>, std::underlying_type_t<E>>
ELMA_FUNC constexpr TE cast_to(E e) noexcept
{
    return static_cast<TE>(e);
}

} // namespace elma