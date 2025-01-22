// Copyright (c) ZeroC, Inc.

#ifndef ICE_ROUTING_TABLE_H
#define ICE_ROUTING_TABLE_H

#include "Ice/Ice.h"
#include "Ice/ObserverHelper.h"
#include "Instrumentation.h"
#include "ProxyVerifier.h"

#include <list>
#include <mutex>

namespace Glacier2
{
    class RoutingTable final
    {
    public:
        RoutingTable(Ice::CommunicatorPtr, std::shared_ptr<ProxyVerifier>);

        void destroy();

        std::shared_ptr<Glacier2::Instrumentation::SessionObserver> updateObserver(
            const std::shared_ptr<Glacier2::Instrumentation::RouterObserver>&,
            const std::string&,
            const Ice::ConnectionPtr&);

        // Returns evicted proxies.
        Ice::ObjectProxySeq add(Ice::ObjectProxySeq, const Ice::Current&);
        std::optional<Ice::ObjectPrx> get(const Ice::Identity&); // Returns nullopt if no proxy can be found.

    private:
        const Ice::CommunicatorPtr _communicator;
        const int _traceLevel;
        const int _maxSize;
        const std::shared_ptr<ProxyVerifier> _verifier;

        struct EvictorEntry;
        using EvictorMap = std::map<Ice::Identity, EvictorEntry>;
        using EvictorQueue = std::list<EvictorMap::iterator>;

        friend struct EvictorEntry;
        struct EvictorEntry
        {
            Ice::ObjectPrx proxy;
            EvictorQueue::iterator pos;
        };

        EvictorMap _map;
        EvictorQueue _queue;

        IceInternal::ObserverHelperT<Glacier2::Instrumentation::SessionObserver> _observer;

        std::mutex _mutex;
    };
}

#endif
