// Copyright (c) ZeroC, Inc.

#ifndef ICE_TUPLE_COMPARE_H
#define ICE_TUPLE_COMPARE_H

#include <tuple>
#include <type_traits>

/// Relational operators for generated structs.
namespace Ice::Tuple
{
    /// Operator less-than for generated structs.
    /// @tparam T The type of the generated struct.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if @p lhs is less than @p rhs, `false` otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    inline bool operator<(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() < rhs.ice_tuple();
    }

    /// Operator less-than or equal to for generated structs.
    /// @tparam T The type of the generated struct.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if @p lhs is less than or equal to @p rhs, `false` otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    inline bool operator<=(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() <= rhs.ice_tuple();
    }

    /// Operator greater-than for generated structs.
    /// @tparam T The type of the generated struct.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if @p lhs is greater than @p rhs, `false` otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    inline bool operator>(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() > rhs.ice_tuple();
    }

    /// Operator greater-than or equal to for generated structs.
    /// @tparam T The type of the generated struct.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if @p lhs is greater than or equal to @p rhs, `false` otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    inline bool operator>=(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() >= rhs.ice_tuple();
    }

    /// Operator equal to for generated structs.
    /// @tparam T The type of the generated struct.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if @p lhs is equal to @p rhs, `false` otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    inline bool operator==(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() == rhs.ice_tuple();
    }

    /// Operator not equal to for generated structs.
    /// @tparam T The type of the generated struct.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if @p lhs is not equal to @p rhs, `false` otherwise.
    template<
        class T,
        std::enable_if_t<
            std::is_member_function_pointer_v<decltype(&T::ice_tuple)> && !std::is_polymorphic_v<T>,
            bool> = true>
    inline bool operator!=(const T& lhs, const T& rhs)
    {
        return lhs.ice_tuple() != rhs.ice_tuple();
    }
}

#endif
