//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ROUTING_TABLE_H
#define ICE_ROUTING_TABLE_H

#include <Ice/Ice.h>
#include <Ice/ObserverHelper.h>
#include <IceUtil/Mutex.h>

#include <Glacier2/ProxyVerifier.h>
#include <Glacier2/Instrumentation.h>

#include <list>

namespace Glacier2
{

class RoutingTable final
{
public:

    RoutingTable(std::shared_ptr<Ice::Communicator>, std::shared_ptr<ProxyVerifier>);

    void destroy();

    std::shared_ptr<Glacier2::Instrumentation::SessionObserver>
    updateObserver(const std::shared_ptr<Glacier2::Instrumentation::RouterObserver>&,
                   const std::string&, const std::shared_ptr<Ice::Connection>&);

    // Returns evicted proxies.
    Ice::ObjectProxySeq add(const Ice::ObjectProxySeq&, const Ice::Current&);
    std::shared_ptr<Ice::ObjectPrx> get(const Ice::Identity&); // Returns null if no proxy can be found.

private:

    const std::shared_ptr<Ice::Communicator> _communicator;
    const int _traceLevel;
    const int _maxSize;
    const std::shared_ptr<ProxyVerifier> _verifier;

    struct EvictorEntry;
    using EvictorMap = std::map<Ice::Identity, std::shared_ptr<EvictorEntry>>;
    using EvictorQueue = std::list<EvictorMap::iterator>;

    friend struct EvictorEntry;
    struct EvictorEntry
    {
        std::shared_ptr<Ice::ObjectPrx> proxy;
        EvictorQueue::iterator pos;
    };

    EvictorMap _map;
    EvictorQueue _queue;

    IceInternal::ObserverHelperT<Glacier2::Instrumentation::SessionObserver> _observer;

    std::mutex _mutex;
};

}

#endif
