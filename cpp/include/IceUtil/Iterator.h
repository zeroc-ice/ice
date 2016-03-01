// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_ITERATOR_H
#define ICE_UTIL_ITERATOR_H

#include <iterator>

namespace IceUtilInternal
{

template<class ForwardIterator>
inline typename ForwardIterator::difference_type
distance(ForwardIterator first, ForwardIterator last)
{
//
// Work-around for a limitation in the standard library provided 
// with the Sun C++ 5.x compilers
#if defined(__SUNPRO_CC) && defined(_RWSTD_NO_CLASS_PARTIAL_SPEC)

    ForwardIterator::difference_type result = 0;
    std::distance(first, last, result);
    return result;
#else
    return ::std::distance(first, last);
#endif
}

}
#endif
