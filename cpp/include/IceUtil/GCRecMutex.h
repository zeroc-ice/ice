// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
