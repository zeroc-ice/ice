// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
