// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    ForwardIterator::difference_type result = 0;
    std::distance(first, last, result);
    return result;
}

#else
#define ice_distance(x,y) std::distance(x,y)
#endif

#endif
