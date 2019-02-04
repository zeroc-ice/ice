//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef INSTRUMENTATION_I_H
#define INSTRUMENTATION_I_H

#include <Ice/MetricsObserverI.h>

#include <IceStorm/Instrumentation.h>
#include <IceStorm/Metrics.h>

namespace IceStorm
{

class TopicObserverI : public IceStorm::Instrumentation::TopicObserver,
                       public IceMX::ObserverT<IceMX::TopicMetrics>
{
public:

    virtual void published();
    virtual void forwarded();
};

class SubscriberObserverI : public IceStorm::Instrumentation::SubscriberObserver,
                            public IceMX::ObserverT<IceMX::SubscriberMetrics>
{
public:

    virtual void queued(int);
    virtual void outstanding(int);
    virtual void delivered(int);
};

class TopicManagerObserverI : public IceStorm::Instrumentation::TopicManagerObserver
{
public:

    TopicManagerObserverI(const IceInternal::MetricsAdminIPtr&);

    virtual void setObserverUpdater(const IceStorm::Instrumentation::ObserverUpdaterPtr&);

    virtual IceStorm::Instrumentation::TopicObserverPtr getTopicObserver(
        const std::string&, const std::string&, const IceStorm::Instrumentation::TopicObserverPtr&);

    virtual IceStorm::Instrumentation::SubscriberObserverPtr getSubscriberObserver(
        const std::string&,
        const std::string&,
        const Ice::ObjectPrx&,
        const IceStorm::QoS&,
        const IceStorm::TopicPrx&,
        IceStorm::Instrumentation::SubscriberState,
        const IceStorm::Instrumentation::SubscriberObserverPtr&);

private:

    const IceInternal::MetricsAdminIPtr _metrics;

    IceMX::ObserverFactoryT<TopicObserverI> _topics;
    IceMX::ObserverFactoryT<SubscriberObserverI> _subscribers;
};
typedef IceUtil::Handle<TopicManagerObserverI> TopicManagerObserverIPtr;

};

#endif
