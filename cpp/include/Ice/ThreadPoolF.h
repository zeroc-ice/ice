// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_THREAD_POOL_F_H
#define ICE_THREAD_POOL_F_H

#include <Ice/Handle.h>

namespace _Ice
{

class ThreadPoolI;
void ICE_API _incRef(ThreadPoolI*);
void ICE_API _decRef(ThreadPoolI*);
typedef _Ice::Handle<ThreadPoolI> ThreadPool;

}

#endif
