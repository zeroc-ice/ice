//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_THREAD_POOL_F_H
#define ICE_THREAD_POOL_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class ThreadPool;
using ThreadPoolPtr = std::shared_ptr<ThreadPool>;

class ThreadPoolCurrent;

}

#endif
