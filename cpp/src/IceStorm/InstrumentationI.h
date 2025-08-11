// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_INSTRUMENTATION_I_H
#define ICESTORM_INSTRUMENTATION_I_H

#include "../Ice/MetricsObserverI.h"
#include "IceStorm/Metrics.h"
#include "Instrumentation.h"

namespace IceStorm
{
    class TopicObserverI final : public IceStorm::Instrumentation::TopicObserver,
                                 public IceMX::ObserverT<IceMX::TopicMetrics>
    {
    public:
        void published() override;
        void forwarded() override;
    };

    class SubscriberObserverI final : public IceStorm::Instrumentation::SubscriberObserver,
                                      public IceMX::ObserverT<IceMX::SubscriberMetrics>
    {
    public:
        void queued(int) override;
        void outstanding(int) override;
        void delivered(int) override;
    };

    class TopicManagerObserverI final : public IceStorm::Instrumentation::TopicManagerObserver
    {
    public:
        TopicManagerObserverI(const std::shared_ptr<IceInternal::MetricsAdminI>&);

        void setObserverUpdater(const std::shared_ptr<IceStorm::Instrumentation::ObserverUpdater>&) override;

        std::shared_ptr<IceStorm::Instrumentation::TopicObserver>
        getTopicObserver(const std::string&, const std::shared_ptr<IceStorm::Instrumentation::TopicObserver>&) override;

        std::shared_ptr<IceStorm::Instrumentation::SubscriberObserver> getSubscriberObserver(
            const std::string&,
            const Ice::ObjectPrx&,
            const IceStorm::QoS&,
            const std::optional<IceStorm::TopicPrx>&,
            IceStorm::Instrumentation::SubscriberState,
            const std::shared_ptr<IceStorm::Instrumentation::SubscriberObserver>&) override;

    private:
        const std::shared_ptr<IceInternal::MetricsAdminI> _metrics;

        IceMX::ObserverFactoryT<TopicObserverI> _topics;
        IceMX::ObserverFactoryT<SubscriberObserverI> _subscribers;
    };
}

#endif
