//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_COMPARABLE_H
#define ICE_COMPARABLE_H

#include <functional>
#include <memory>
#include <optional>

namespace Ice
{

/**
 * Compares the contents of two smart pointers.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the contents are equal, false otherwise.
 */
template<typename T, typename U>
inline bool targetEqualTo(const T& lhs, const U& rhs)
{
    if(lhs && rhs)
    {
        return *lhs == *rhs;
    }
    else
    {
        return !lhs && !rhs;
    }
}

/**
 * Compares the contents of two smart pointers.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the left-hand side compares less than the right-hand side, false otherwise.
 */
template<typename T, typename U>
inline bool targetLess(const T& lhs, const U& rhs)
{
    if(lhs && rhs)
    {
        return *lhs < *rhs;
    }
    else
    {
        return !lhs && rhs;
    }
}

/**
 * Compares the contents of two smart pointers.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the left-hand side compares greater than the right-hand side, false otherwise.
 */
template<typename T, typename U>
inline bool targetGreater(const T& lhs, const U& rhs)
{
    return targetLess(rhs, lhs);
}

/**
 * Compares the contents of two smart pointers.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the left-hand side compares less than or equal to the right-hand side, false otherwise.
 */
template<typename T, typename U>
inline bool targetLessEqual(const T& lhs, const U& rhs)
{
    return !targetGreater(lhs, rhs);
}

/**
 * Compares the contents of two smart pointers.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the left-hand side compares greater than or equal to the right-hand side, false otherwise.
 */
template<typename T, typename U>
inline bool targetGreaterEqual(const T& lhs, const U& rhs)
{
    return !targetLess(lhs, rhs);
}

/**
 * Compares the contents of two smart pointers.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the contents are not equal, false otherwise.
 */
template<typename T, typename U>
inline bool targetNotEqualTo(const T& lhs, const U& rhs)
{
    return !targetEqualTo(lhs, rhs);
}

/**
 * Functor class that compares the contents of two smart pointers (or similar) of the given type using the given
 * comparator. It provides partial specializations for std::shared and std::optional.
 * \headerfile Ice/Ice.h
 */
template<typename T, template<typename> class Compare>
struct TargetCompare;

// Partial specialization for std::shared_ptr.
template<typename T, template<typename> class Compare>
struct TargetCompare<std::shared_ptr<T>, Compare>
{
    /**
     * Executes the functor to compare the contents of two smart pointers.
     * @return True if the contents satisfy the given comparator, false otherwise.
     */
    bool operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) const
    {
        if(lhs && rhs)
        {
            return Compare<T>()(*lhs, *rhs);
        }
        else
        {
            return Compare<bool>()(static_cast<const bool>(lhs), static_cast<const bool>(rhs));
        }
    }
};

// Partial specialization for std::optional.
template<typename T, template<typename> class Compare>
struct TargetCompare<std::optional<T>, Compare>
{
    /**
     * Executes the functor to compare the contents of two optionals.
     * @return True if the contents satisfy the given comparator, false otherwise.
     */
    bool operator()(const std::optional<T>& lhs, const std::optional<T>& rhs) const
    {
        if(lhs && rhs)
        {
            return Compare<T>()(*lhs, *rhs);
        }
        else
        {
            return Compare<bool>()(static_cast<const bool>(lhs), static_cast<const bool>(rhs));
        }
    }
};

//
// Relational operators for generated structs and classes
//

/**
 * Relational operator for generated structs and classes.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the left-hand side compares less than the right-hand side, false otherwise.
 */
template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator<(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() < rhs.ice_tuple();
}

/**
 * Relational operator for generated structs and classes.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the left-hand side compares less than or equal to the right-hand side, false otherwise.
 */
template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator<=(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() <= rhs.ice_tuple();
}

/**
 * Relational operator for generated structs and classes.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the left-hand side compares greater than the right-hand side, false otherwise.
 */
template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator>(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() > rhs.ice_tuple();
}

/**
 * Relational operator for generated structs and classes.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the left-hand side compares greater than or equal to the right-hand side, false otherwise.
 */
template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator>=(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() >= rhs.ice_tuple();
}

/**
 * Relational operator for generated structs and classes.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the left-hand side compares equal to the right-hand side, false otherwise.
 */
template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator==(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() == rhs.ice_tuple();
}

/**
 * Relational operator for generated structs and classes.
 * @param lhs The left-hand side.
 * @param rhs The right-hand side.
 * @return True if the left-hand side is not equal to the right-hand side, false otherwise.
 */
template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator!=(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() != rhs.ice_tuple();
}

}

#endif
