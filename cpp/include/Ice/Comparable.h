// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_COMPARABLE_H
#define ICE_COMPARABLE_H

#include <functional>

namespace Ice
{

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

template<typename T, typename U>
inline bool targetGreater(const T& lhs, const U& rhs)
{
    return targetLess(rhs, lhs);
}

template<typename T, typename U>
inline bool targetLessEqual(const T& lhs, const U& rhs)
{
    return !targetGreater(lhs, rhs);
}

template<typename T, typename U>
inline bool targetGreaterEqual(const T& lhs, const U& rhs)
{
    return !targetLess(lhs, rhs);
}

template<typename T, typename U>
inline bool targetNotEqualTo(const T& lhs, const U& rhs)
{
    return !targetEqualTo(lhs, rhs);
}

#ifdef ICE_CPP11_MAPPING

template<typename T, template<typename> class Compare>
struct TargetCompare
{
    bool operator()(const T& lhs, const T& rhs) const
    {
        if(lhs && rhs)
        {
            return Compare<typename T::element_type>()(*lhs, *rhs);
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

template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator<(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() < rhs.ice_tuple();
}

template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator<=(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() <= rhs.ice_tuple();
}

template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator>(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() > rhs.ice_tuple();
}

template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator>=(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() >= rhs.ice_tuple();
}

template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator==(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() == rhs.ice_tuple();
}

template<class C, typename = std::enable_if<std::is_member_function_pointer<decltype(&C::ice_tuple)>::value>>
bool operator!=(const C& lhs, const C& rhs)
{
   return lhs.ice_tuple() != rhs.ice_tuple();
}

#endif

}

#endif
