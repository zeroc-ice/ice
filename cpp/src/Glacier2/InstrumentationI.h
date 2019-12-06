//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef INSTRUMENTATION_I_H
#define INSTRUMENTATION_I_H

#include <Ice/MetricsObserverI.h>

#include <Glacier2/Instrumentation.h>
#include <Glacier2/Metrics.h>

namespace Glacier2
{

class SessionObserverI final : public Glacier2::Instrumentation::SessionObserver,
                               public IceMX::ObserverT<IceMX::SessionMetrics>
{
public:

    void forwarded(bool) override;
    void queued(bool) override;
    void overridden(bool) override;
    void routingTableSize(int) override;
};

class RouterObserverI final : public Glacier2::Instrumentation::RouterObserver
{
public:

    RouterObserverI(std::shared_ptr<IceInternal::MetricsAdminI>, const std::string&);

    void setObserverUpdater(const std::shared_ptr<Glacier2::Instrumentation::ObserverUpdater>&) override;

    std::shared_ptr<Glacier2::Instrumentation::SessionObserver> getSessionObserver(const std::string&,
        const std::shared_ptr<Ice::Connection>&, int,
        const std::shared_ptr<Glacier2::Instrumentation::SessionObserver>&) override;

private:

    const std::shared_ptr<IceInternal::MetricsAdminI> _metrics;
    const std::string _instanceName;

    IceMX::ObserverFactoryT<SessionObserverI> _sessions;
};

}

#endif
