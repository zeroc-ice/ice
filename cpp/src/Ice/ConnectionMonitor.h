// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_CONNECTION_MONITOR_H
#define ICE_CONNECTION_MONITOR_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Thread.h>
#include <Ice/ConnectionMonitorF.h>
#include <Ice/ConnectionF.h>
#include <Ice/InstanceF.h>
#include <set>

namespace IceInternal
{

class ConnectionMonitor : public ::IceUtil::Thread, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    void destroy();

    void add(const ConnectionPtr&);
    void remove(const ConnectionPtr&);

private:

    ConnectionMonitor(const InstancePtr&, int);
    virtual ~ConnectionMonitor();
    friend class Instance;

    virtual void run();

    InstancePtr _instance;
    const int _interval;
    std::set<ConnectionPtr> _connections;
};

}

#endif
