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

class RoutingTable;
typedef IceUtil::Handle<RoutingTable> RoutingTablePtr;

class RoutingTable : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    RoutingTable(const Ice::CommunicatorPtr&, const ProxyVerifierPtr&);

    void destroy();

    Glacier2::Instrumentation::SessionObserverPtr
    updateObserver(const Glacier2::Instrumentation::RouterObserverPtr&, const std::string&, const Ice::ConnectionPtr&);

    // Returns evicted proxies.
    Ice::ObjectProxySeq add(const Ice::ObjectProxySeq&, const Ice::Current&);
    Ice::ObjectPrx get(const Ice::Identity&); // Returns null if no proxy can be found.

private:

    const Ice::CommunicatorPtr _communicator;
    const int _traceLevel;
    const int _maxSize;
    const ProxyVerifierPtr _verifier;

    struct EvictorEntry;
    typedef IceUtil::Handle<EvictorEntry> EvictorEntryPtr;

    typedef std::map<Ice::Identity, EvictorEntryPtr> EvictorMap;
    typedef std::list<EvictorMap::iterator> EvictorQueue;

    friend struct EvictorEntry;
    struct EvictorEntry : public IceUtil::Shared
    {
        Ice::ObjectPrx proxy;
        EvictorQueue::iterator pos;
    };

    EvictorMap _map;
    EvictorQueue _queue;

    IceInternal::ObserverHelperT<Glacier2::Instrumentation::SessionObserver> _observer;
};

}

#endif
