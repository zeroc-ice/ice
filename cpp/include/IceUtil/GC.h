// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GC_H
#define ICE_GC_H

#include <IceUtil/Config.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>

namespace IceUtil
{

struct ICE_UTIL_API GCStats
{
    int examined;
    int collected;
    Time time;
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
