// Copyright (c) ZeroC, Inc.

#ifndef TYPE_CONTEXT_H
#define TYPE_CONTEXT_H

#include <cstdint>
#include <type_traits>

namespace Slice
{
    enum class TypeContext : std::uint8_t
    {
        None = 0,
        UseWstring = 1,
        MarshalParam = 2,
        UnmarshalParamZeroCopy = 4
    };

    inline constexpr TypeContext operator|(TypeContext lhs, TypeContext rhs) noexcept
    {
        return static_cast<TypeContext>(
            static_cast<std::underlying_type_t<TypeContext>>(lhs) |
            static_cast<std::underlying_type_t<TypeContext>>(rhs));
    }

    inline constexpr TypeContext operator&(TypeContext lhs, TypeContext rhs) noexcept
    {
        return static_cast<TypeContext>(
            static_cast<std::underlying_type_t<TypeContext>>(lhs) &
            static_cast<std::underlying_type_t<TypeContext>>(rhs));
    }
}

#endif
