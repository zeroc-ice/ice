//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_ITERATOR_H
#define ICE_UTIL_ITERATOR_H

#include <iterator>

namespace IceUtilInternal
{

template<class ForwardIterator>
inline typename ForwardIterator::difference_type
distance(ForwardIterator first, ForwardIterator last)
{
    return ::std::distance(first, last);
}

}
#endif
