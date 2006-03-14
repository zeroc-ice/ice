// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_THREAD_POOL_F_H
#define ICE_THREAD_POOL_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class ThreadPool;
void incRef(ThreadPool*);
void decRef(ThreadPool*);
typedef Handle<ThreadPool> ThreadPoolPtr;

}

#endif
