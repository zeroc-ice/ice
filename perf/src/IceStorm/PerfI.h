// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef PERF_I_H
#define PERF_I_H

#include <Perf.h>
#include <IceUtil/Mutex.h>

class PingI : public Perf::Ping, IceUtil::Mutex
{
public:

    PingI(int, int);
    virtual void tickVoid(long long, const Ice::Current&);
    virtual void tick(long long, Perf::AEnum, int, const Perf::AStruct&, const Ice::Current&);

private:

    void started();
    bool stopped();
    void add(long long);
    void calc();

    int _nPublishers;
    int _nStartedPublishers;
    int _nStoppedPublishers;
    int _nExpectedTicks;
    int _nReceived;
    std::vector<int> _results;
    IceUtil::Time _startTime;
    IceUtil::Time _stopTime;
};

#endif
