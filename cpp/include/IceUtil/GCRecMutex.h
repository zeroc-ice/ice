// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GC_REC_MUTEX_H
#define ICE_GC_REC_MUTEX_H

#include <IceUtil/RecMutex.h>

namespace IceUtil
{

class ICE_UTIL_API GCRecMutex
{
public:

    GCRecMutex();
    ~GCRecMutex();
    RecMutex* _m;
};

extern ICE_UTIL_API GCRecMutex gcRecMutex;

class ICE_UTIL_API GCRecMutexInit
{
public:

    GCRecMutexInit();
    ~GCRecMutexInit();
};

static GCRecMutexInit gcRecMutexInit;

}

#endif
