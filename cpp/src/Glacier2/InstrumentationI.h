// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef INSTRUMENTATION_I_H
#define INSTRUMENTATION_I_H

#include <Ice/MetricsObserverI.h>

#include <Glacier2/Instrumentation.h>
#include <Glacier2/Metrics.h>

namespace Glacier2
{

class SessionObserverI : public Glacier2::Instrumentation::SessionObserver, 
                         public IceMX::ObserverT<IceMX::SessionMetrics>
{
public:

    virtual void forwarded(bool);
    virtual void queued(bool);
    virtual void overridden(bool);
    virtual void routingTableSize(int);
};

class RouterObserverI : public Glacier2::Instrumentation::RouterObserver
{
public:

    RouterObserverI(const IceInternal::MetricsAdminIPtr&, const std::string&);

    virtual void setObserverUpdater(const Glacier2::Instrumentation::ObserverUpdaterPtr&);
 
    virtual Glacier2::Instrumentation::SessionObserverPtr getSessionObserver(
        const std::string&, const Ice::ConnectionPtr&, int, const Glacier2::Instrumentation::SessionObserverPtr&);

private:

    const IceInternal::MetricsAdminIPtr _metrics;
    const std::string _instanceName;

    IceMX::ObserverFactoryT<SessionObserverI> _sessions;
};
typedef IceUtil::Handle<RouterObserverI> RouterObserverIPtr;

};

#endif
