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

#include <IceUtil/GCRecMutex.h>

using namespace IceUtil;

GCRecMutex IceUtil::gcRecMutex;

static int gcInitCount = 0;

GCRecMutex::GCRecMutex()
{
    if(gcInitCount++ == 0)
    {
	_m = new RecMutex;
    }
}

GCRecMutex::~GCRecMutex()
{
    if(--gcInitCount == 0)
    {
	delete _m;
    }
}

GCRecMutexInit::GCRecMutexInit()
{
    if(gcInitCount++ == 0)
    {
	gcRecMutex._m = new RecMutex;
    }
}

GCRecMutexInit::~GCRecMutexInit()
{
    if(--gcInitCount == 0)
    {
	delete gcRecMutex._m;
    }
}
