// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_ALGORITHM_H
#define ICE_UTIL_ALGORITHM_H

#include <algorithm>


// Work-around for a limitation in the standard library provided 
// with the Sun C++ 5.x compilers
#if defined(__SUNPRO_CC) && defined(_RWSTD_NO_CLASS_PARTIAL_SPEC)

template<class InputIterator, class Predicate>
inline typename InputIterator::difference_type
ice_count_if(InputIterator first, InputIterator last, Predicate pred)
{
    InputIterator::difference_type result = 0;
    std::count_if(first, last, pred, result);
    return result;
}

#else
#define ice_count_if(x,y,z) std::count_if(x,y,z)
#endif

#endif
