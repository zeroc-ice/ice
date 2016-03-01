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

namespace Ice
{

template<typename T, typename U>
inline bool targetEquals(const T& lhs, const U& rhs)
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

template<typename T>
struct TargetEquals
{
    bool operator()(const T& lhs, const T& rhs) const
    {
        return targetEquals(lhs, rhs);
    }
};


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

template<typename T>
struct TargetLess
{
    bool operator()(const T& lhs, const T& rhs) const
    {
        return targetLess(lhs, rhs);
    }
};

}

#endif
