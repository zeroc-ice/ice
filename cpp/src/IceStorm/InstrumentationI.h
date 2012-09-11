// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef INSTRUMENTATION_I_H
#define INSTRUMENTATION_I_H

#include <Ice/MetricsObserverI.h>

#include <IceStorm/Instrumentation.h>
#include <IceStorm/Metrics.h>

namespace IceMX
{

class TopicObserverI : public IceStorm::Instrumentation::TopicObserver, public ObserverT<TopicMetrics>
{
public:

    virtual void published();
    virtual void forwarded();
};

class SubscriberObserverI : public IceStorm::Instrumentation::SubscriberObserver, public ObserverT<SubscriberMetrics>
{
public:

    virtual void queued(int);
    virtual void outstanding(int);
    virtual void delivered(int);
};

class TopicManagerObserverI : public IceStorm::Instrumentation::TopicManagerObserver
{
public:

    TopicManagerObserverI(const MetricsAdminIPtr&);

    virtual void setObserverUpdater(const IceStorm::Instrumentation::ObserverUpdaterPtr&);
 
    virtual IceStorm::Instrumentation::TopicObserverPtr getTopicObserver(
        const std::string&, const std::string&, const IceStorm::Instrumentation::TopicObserverPtr&);
    
    virtual IceStorm::Instrumentation::SubscriberObserverPtr getSubscriberObserver(
        const std::string&,
        const std::string&,
        const Ice::ObjectPrx&,
        const IceStorm::QoS&,
        bool,
        const IceStorm::Instrumentation::SubscriberObserverPtr&);

private:

    const MetricsAdminIPtr _metrics;

    ObserverFactoryT<TopicObserverI> _topics;
    ObserverFactoryT<SubscriberObserverI> _subscribers;
};
typedef IceUtil::Handle<TopicManagerObserverI> TopicManagerObserverIPtr;

};

#endif
