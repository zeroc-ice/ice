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

#ifndef ICE_UTIL_ITERATOR_H
#define ICE_UTIL_ITERATOR_H

#include <iterator>


// Work-around for a limitation in the standard library provided 
// with the Sun C++ 5.x compilers
#if defined(__SUNPRO_CC) && defined(_RWSTD_NO_CLASS_PARTIAL_SPEC)

template<class ForwardIterator>
inline typename ForwardIterator::difference_type
ice_distance(ForwardIterator first, ForwardIterator last)
{
    ForwardIterator::difference_type result;
    std::distance(first, last, result);
    return result;
}

#else
#define ice_distance(x,y) std::distance(x,y)
#endif

#endif
