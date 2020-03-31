//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UNIQUE_PTR_H
#define ICE_UNIQUE_PTR_H

#include <Ice/Config.h>

namespace IceInternal
{

template<typename T>
using UniquePtr = std::unique_ptr<T>;

}

#endif
