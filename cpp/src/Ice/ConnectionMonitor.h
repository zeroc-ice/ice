// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTION_MONITOR_H
#define ICE_CONNECTION_MONITOR_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Thread.h>
#include <Ice/ConnectionMonitorF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/InstanceF.h>
#include <set>

namespace IceInternal
{

class ConnectionMonitor : public ::IceUtil::Thread, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    void destroy();

    void add(const Ice::ConnectionIPtr&);
    void remove(const Ice::ConnectionIPtr&);

private:

    ConnectionMonitor(const InstancePtr&, Ice::Int);
    virtual ~ConnectionMonitor();
    friend class Instance;

    virtual void run();

    InstancePtr _instance;
    const IceUtil::Time _interval;
    std::set<Ice::ConnectionIPtr> _connections;
};

}

#endif
