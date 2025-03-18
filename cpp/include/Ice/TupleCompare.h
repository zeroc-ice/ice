// Copyright (c) ZeroC, Inc.

#ifndef ICE_TUPLE_COMPARE_H
#define ICE_TUPLE_COMPARE_H

#include <tuple>
#include <type_traits>

namespace Ice::Tuple
{
    /// Relational operator for generated structs.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return @c true if the left-hand side compares less than the right-hand side, @c false otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    bool operator<(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() < rhs.ice_tuple();
    }

    /// Relational operator for generated structs.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return @c true if the left-hand side compares less than or equal to the right-hand side, @c false otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    bool operator<=(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() <= rhs.ice_tuple();
    }

    /// Relational operator for generated structs.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return @c true if the left-hand side compares greater than the right-hand side, @c false otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    bool operator>(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() > rhs.ice_tuple();
    }

    /// Relational operator for generated structs.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return @c true if the left-hand side compares greater than or equal to the right-hand side, @c false otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    bool operator>=(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() >= rhs.ice_tuple();
    }

    /// Relational operator for generated structs.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return @c true if the left-hand side compares equal to the right-hand side, @c false otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    bool operator==(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() == rhs.ice_tuple();
    }

    /// Relational operator for generated structs.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return @c true if the left-hand side is not equal to the right-hand side, @c false otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    bool operator!=(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() != rhs.ice_tuple();
    }
}

#endif
