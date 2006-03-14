// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
#include <Ice/Config.h>

namespace IceInternal
{

struct ICE_API GCStats
{
    int examined;
    int collected;
    IceUtil::Time time;
};

class GC : public ::IceUtil::Thread, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    typedef void (*StatsCallback)(const ::IceInternal::GCStats&);

    ICE_API GC(int, StatsCallback);
    ICE_API virtual ~GC();
    ICE_API virtual void run();
    ICE_API void stop();
    ICE_API void collectGarbage();

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
