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

#ifndef ICE_GC_H
#define ICE_GC_H

#include <Ice/Config.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>

namespace IceUtil
{

struct ICE_UTIL_API GCStats
{
    int examined;
    int collected;
    double msec;
};

class GC : public ::IceUtil::Thread, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    typedef void (*StatsCallback)(const ::IceUtil::GCStats&);

    ICE_UTIL_API GC(int, StatsCallback);
    ICE_UTIL_API virtual ~GC();
    ICE_UTIL_API virtual void run();
    ICE_UTIL_API void stop();
    ICE_UTIL_API void collectGarbage();

private:

    enum State { NotStarted, Started, Stopping, Stopped };
    State _state;
    bool _collecting;
    int _interval;
    StatsCallback _statsCallback;
    static int _numCollectors;
};

}

#endif
