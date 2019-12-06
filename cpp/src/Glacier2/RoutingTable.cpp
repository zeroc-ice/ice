//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/RoutingTable.h>
#include <Glacier2/Instrumentation.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::RoutingTable::RoutingTable(shared_ptr<Communicator> communicator, shared_ptr<ProxyVerifier> verifier) :
    _communicator(move(communicator)),
    _traceLevel(_communicator->getProperties()->getPropertyAsInt("Glacier2.Trace.RoutingTable")),
    _maxSize(_communicator->getProperties()->getPropertyAsIntWithDefault("Glacier2.RoutingTable.MaxSize", 1000)),
    _verifier(move(verifier))
{
}

void
Glacier2::RoutingTable::destroy()
{
    lock_guard<mutex> lg(_mutex);
    if(_observer)
    {
        _observer->routingTableSize(-static_cast<int>(_map.size()));
    }
    _observer.detach();
}

shared_ptr<Glacier2::Instrumentation::SessionObserver>
Glacier2::RoutingTable::updateObserver(const shared_ptr<Glacier2::Instrumentation::RouterObserver>& obsv,
                                       const string& userId,
                                       const shared_ptr<Ice::Connection>& connection)
{
    lock_guard<mutex> lg(_mutex);
    _observer.attach(obsv->getSessionObserver(userId, connection, static_cast<int>(_map.size()), _observer.get()));
    return _observer.get();
}

ObjectProxySeq
Glacier2::RoutingTable::add(const ObjectProxySeq& unfiltered, const Current& current)
{
    lock_guard<mutex> lg(_mutex);

    size_t sz = _map.size();

    //
    // We 'pre-scan' the list, applying our validation rules. The
    // ensures that our state is not modified if this operation results
    // in a rejection.
    //
    ObjectProxySeq proxies;
    for(const auto& prx : unfiltered)
    {
        if(!prx) // We ignore null proxies.
        {
            continue;
        }

        if(!_verifier->verify(prx))
        {
            current.con->close(ConnectionClose::Forcefully);
            throw ObjectNotExistException(__FILE__, __LINE__);
        }
        auto proxy = prx->ice_twoway()->ice_secure(false)->ice_facet(""); // We add proxies in default form.
        proxies.push_back(proxy);
    }

    ObjectProxySeq evictedProxies;
    for(const auto& proxy : proxies)
    {
        EvictorMap::iterator p = _map.find(proxy->ice_getIdentity());

        if(p == _map.end())
        {
            if(_traceLevel == 1 || _traceLevel >= 3)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << "adding proxy to routing table:\n" << _communicator->proxyToString(proxy);
            }

            auto entry = make_shared<EvictorEntry>();
            p = _map.insert(_map.begin(), {proxy->ice_getIdentity(), entry});
            EvictorQueue::iterator q = _queue.insert(_queue.end(), p);
            entry->proxy = proxy;
            entry->pos = q;
        }
        else
        {
            if(_traceLevel == 1 || _traceLevel >= 3)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << "proxy already in routing table:\n" << _communicator->proxyToString(proxy);
            }

            auto entry = p->second;
            _queue.erase(entry->pos);
            EvictorQueue::iterator q = _queue.insert(_queue.end(), p);
            entry->pos = q;
        }

        while(static_cast<int>(_map.size()) > _maxSize)
        {
            p = _queue.front();

            if(_traceLevel >= 2)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << "evicting proxy from routing table:\n" << _communicator->proxyToString(p->second->proxy);
            }

            evictedProxies.push_back(p->second->proxy);

            _map.erase(p);
            _queue.pop_front();
        }
    }

    if(_observer)
    {
        _observer->routingTableSize(static_cast<int>(_map.size()) - static_cast<int>(sz));
    }

    return evictedProxies;
}

shared_ptr<ObjectPrx>
Glacier2::RoutingTable::get(const Identity& ident)
{
    if(ident.name.empty())
    {
        return 0;
    }

    lock_guard<mutex> lg(_mutex);

    EvictorMap::iterator p = _map.find(ident);

    if(p == _map.end())
    {
        return 0;
    }
    else
    {
        auto entry = p->second;
        _queue.erase(entry->pos);
        EvictorQueue::iterator q = _queue.insert(_queue.end(), p);
        entry->pos = q;

        return entry->proxy;
    }
}
