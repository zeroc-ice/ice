// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/GCRecMutex.h>

namespace IceUtil
{
GCRecMutex gcRecMutex;
}

using namespace IceUtil;

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
