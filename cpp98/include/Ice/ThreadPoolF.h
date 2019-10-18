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
ICE_API IceUtil::Shared* upCast(ThreadPool*);
typedef Handle<ThreadPool> ThreadPoolPtr;

class ThreadPoolCurrent;

}

#endif
