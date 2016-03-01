// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ACM_H
#define ICE_ACM_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Timer.h>
#include <Ice/ACMF.h>
#include <Ice/Connection.h>
#include <Ice/ConnectionIF.h>
#include <Ice/InstanceF.h>
#include <Ice/PropertiesF.h>
#include <Ice/LoggerF.h>
#include <set>

namespace IceInternal
{

class ACMConfig
{
public:

    ACMConfig(bool = false);
    ACMConfig(const Ice::PropertiesPtr&, const Ice::LoggerPtr&, const std::string&, const ACMConfig&);

    IceUtil::Time timeout;
    Ice::ACMHeartbeat heartbeat;
    Ice::ACMClose close;
};

class ACMMonitor : public IceUtil::TimerTask
{
public:

    virtual void add(const Ice::ConnectionIPtr&) = 0;
    virtual void remove(const Ice::ConnectionIPtr&) = 0;
    virtual void reap(const Ice::ConnectionIPtr&) = 0;

    virtual ACMMonitorPtr acm(const IceUtil::Optional<int>&, 
                              const IceUtil::Optional<Ice::ACMClose>&, 
                              const IceUtil::Optional<Ice::ACMHeartbeat>&) = 0;
    virtual Ice::ACM getACM() = 0;
};

class FactoryACMMonitor : public ACMMonitor, public ::IceUtil::Mutex
{
public:

    FactoryACMMonitor(const InstancePtr&, const ACMConfig&);
    virtual ~FactoryACMMonitor();

    virtual void add(const Ice::ConnectionIPtr&);
    virtual void remove(const Ice::ConnectionIPtr&);
    virtual void reap(const Ice::ConnectionIPtr&);

    virtual ACMMonitorPtr acm(const IceUtil::Optional<int>&, 
                              const IceUtil::Optional<Ice::ACMClose>&, 
                              const IceUtil::Optional<Ice::ACMHeartbeat>&);
    virtual Ice::ACM getACM();

    void destroy();
    void swapReapedConnections(std::vector<Ice::ConnectionIPtr>&);

private:

    friend class ConnectionACMMonitor;
    void handleException(const std::exception&);
    void handleException();

    virtual void runTimerTask();

    InstancePtr _instance;
    const ACMConfig _config;

    std::vector<std::pair<Ice::ConnectionIPtr, bool> > _changes;
    std::set<Ice::ConnectionIPtr> _connections;
    std::vector<Ice::ConnectionIPtr> _reapedConnections;
};

class ConnectionACMMonitor : public ACMMonitor, public ::IceUtil::Mutex
{
public:

    ConnectionACMMonitor(const FactoryACMMonitorPtr&, const IceUtil::TimerPtr&, const ACMConfig&);
    virtual ~ConnectionACMMonitor();

    virtual void add(const Ice::ConnectionIPtr&);
    virtual void remove(const Ice::ConnectionIPtr&);
    virtual void reap(const Ice::ConnectionIPtr&);

    virtual ACMMonitorPtr acm(const IceUtil::Optional<int>&, 
                              const IceUtil::Optional<Ice::ACMClose>&, 
                              const IceUtil::Optional<Ice::ACMHeartbeat>&);
    virtual Ice::ACM getACM();

private:

    virtual void runTimerTask();

    const FactoryACMMonitorPtr _parent;
    const IceUtil::TimerPtr _timer;
    const ACMConfig _config;

    Ice::ConnectionIPtr _connection;
};

}

#endif
