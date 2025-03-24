// Copyright (c) ZeroC, Inc.

#ifndef ICE_TARGET_COMPARE_H
#define ICE_TARGET_COMPARE_H

namespace Ice
{
    /// Compares the contents of two smart pointers.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if the contents are equal, `false` otherwise.
    template<typename T, typename U> inline bool targetEqualTo(const T& lhs, const U& rhs)
    {
        if (lhs && rhs)
        {
            return *lhs == *rhs;
        }
        else
        {
            return !lhs && !rhs;
        }
    }

    /// Compares the contents of two smart pointers.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if the left-hand side compares less than the right-hand side, `false` otherwise.
    template<typename T, typename U> inline bool targetLess(const T& lhs, const U& rhs)
    {
        if (lhs && rhs)
        {
            return *lhs < *rhs;
        }
        else
        {
            return !lhs && rhs;
        }
    }

    /// Compares the contents of two smart pointers.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if the left-hand side compares greater than the right-hand side, `false` otherwise.
    template<typename T, typename U> inline bool targetGreater(const T& lhs, const U& rhs)
    {
        return targetLess(rhs, lhs);
    }

    /// Compares the contents of two smart pointers.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if the left-hand side compares less than or equal to the right-hand side, `false` otherwise.
    template<typename T, typename U> inline bool targetLessEqual(const T& lhs, const U& rhs)
    {
        return !targetGreater(lhs, rhs);
    }

    /// Compares the contents of two smart pointers.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if the left-hand side compares greater than or equal to the right-hand side, `false` otherwise.
    template<typename T, typename U> inline bool targetGreaterEqual(const T& lhs, const U& rhs)
    {
        return !targetLess(lhs, rhs);
    }

    /// Compares the contents of two smart pointers.
    /// @param lhs The left-hand side.
    /// @param rhs The right-hand side.
    /// @return `true` if the contents are not equal, `false` otherwise.
    template<typename T, typename U> inline bool targetNotEqualTo(const T& lhs, const U& rhs)
    {
        return !targetEqualTo(lhs, rhs);
    }

    /// Functor class that compares the contents of two smart pointers (or similar) of the given type using the given
    /// comparator.
    template<typename T, template<typename> class Compare> struct TargetCompare
    {
        /// Executes the functor to compare the contents of two smart pointers.
        /// @return `true` if the contents satisfy the given comparator, `false` otherwise.
        bool operator()(const T& lhs, const T& rhs) const
        {
            if (lhs && rhs)
            {
                return Compare<typename T::element_type>()(*lhs, *rhs);
            }
            else
            {
                return Compare<bool>()(static_cast<const bool>(lhs), static_cast<const bool>(rhs));
            }
        }
    };
}

#endif
