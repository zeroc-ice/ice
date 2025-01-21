// Copyright (c) ZeroC, Inc.

#ifndef ICE_ENDIAN_H
#define ICE_ENDIAN_H

#if __has_include(<bit>)
#    include <bit>
#endif

#ifndef __cpp_lib_endian

// NOLINTBEGIN(cert-dcl58-cpp)

// Provide a minimal implementation of std::endian for C++17.
namespace std
{
#    ifdef _MSC_VER
    // Always little-endian on Windows.
    enum class endian
    {
        little = 0,
        big = 1,
        native = little
    };
#    else
    // Use macros predefined by gcc/clang to determine the endianness.
    enum class endian
    {
        little = __ORDER_LITTLE_ENDIAN__,
        big = __ORDER_BIG_ENDIAN__,
        native = __BYTE_ORDER__
    };
#    endif
}

// NOLINTEND(cert-dcl58-cpp)

#endif

#endif
