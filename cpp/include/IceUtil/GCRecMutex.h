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

#ifndef ICE_GCRECMUTEX_H
#define ICE_GCRECMUTEX_H

#include <IceUtil/RecMutex.h>

namespace IceUtil
{

class GCRecMutex
{
public:

    GCRecMutex();
    ~GCRecMutex();
    RecMutex* _m;
};

extern GCRecMutex gcRecMutex;

class GCRecMutexInit
{
public:

    GCRecMutexInit();
    ~GCRecMutexInit();
};

static GCRecMutexInit gcRecMutexInit;

}

#endif
