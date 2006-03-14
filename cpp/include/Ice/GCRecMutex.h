// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GC_REC_MUTEX_H
#define ICE_GC_REC_MUTEX_H

#include <IceUtil/RecMutex.h>
#include <Ice/Config.h>

namespace IceInternal
{

class ICE_API GCRecMutex
{
public:

    GCRecMutex();
    ~GCRecMutex();
    ::IceUtil::RecMutex* _m;
};

extern ICE_API GCRecMutex gcRecMutex;

class ICE_API GCRecMutexInit
{
public:

    GCRecMutexInit();
    ~GCRecMutexInit();
};

static GCRecMutexInit gcRecMutexInit;

}

#endif
