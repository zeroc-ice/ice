// Copyright (c) ZeroC, Inc.

#include "RoutingTable.h"
#include "Instrumentation.h"

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::RoutingTable::RoutingTable(CommunicatorPtr communicator, shared_ptr<ProxyVerifier> verifier, int maxSize)
    : _communicator(std::move(communicator)),
      _traceLevel(_communicator->getProperties()->getIcePropertyAsInt("Glacier2.Trace.RoutingTable")),
      _maxSize(maxSize),
      _verifier(std::move(verifier))
{
}

void
Glacier2::RoutingTable::destroy()
{
    lock_guard<mutex> lock(_mutex);
    if (_observer)
    {
        _observer->routingTableSize(-static_cast<int>(_map.size()));
    }
    _observer.detach();
}

shared_ptr<Glacier2::Instrumentation::SessionObserver>
Glacier2::RoutingTable::updateObserver(
    const shared_ptr<Glacier2::Instrumentation::RouterObserver>& obsv,
    const string& userId,
    const shared_ptr<Ice::Connection>& connection)
{
    lock_guard<mutex> lock(_mutex);
    _observer.attach(obsv->getSessionObserver(userId, connection, static_cast<int>(_map.size()), _observer.get()));
    return _observer.get();
}

ObjectProxySeq
Glacier2::RoutingTable::add(
    ObjectProxySeq unfiltered, // NOLINT(performance-unnecessary-value-param)
    const Current& current)
{
    lock_guard<mutex> lock(_mutex);

    size_t sz = _map.size();

    //
    // We 'pre-scan' the list, applying our validation rules. The
    // ensures that our state is not modified if this operation results
    // in a rejection.
    //
    vector<ObjectPrx> proxies;
    for (const auto& prx : unfiltered)
    {
        if (!prx) // We ignore null proxies.
        {
            continue;
        }

        if (!_verifier->verify(*prx))
        {
            current.con->abort();
            throw ObjectNotExistException{__FILE__, __LINE__};
        }

        proxies.push_back(prx->ice_twoway()->ice_facet("")); // We add proxies in default form.
    }

    ObjectProxySeq evictedProxies;
    for (const auto& proxy : proxies)
    {
        auto p = _map.find(proxy->ice_getIdentity());

        if (p == _map.end())
        {
            if (_traceLevel >= 1)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << "adding proxy to routing table:\n" << proxy;
            }

            p = _map.insert(_map.begin(), make_pair(proxy->ice_getIdentity(), EvictorEntry{proxy, _queue.end()}));
            auto q = _queue.insert(_queue.end(), p);
            p->second.pos = q;
        }
        else
        {
            auto& entry = p->second;

            // Two distinct proxies with the same identity can't both be routed. We accept duplicate registrations
            // when they are identical.
            if (entry.proxy != proxy)
            {
                throw DispatchException{
                    __FILE__,
                    __LINE__,
                    ReplyStatus::NotSupported,
                    "cannot route two proxies with the same identity '" +
                        _communicator->identityToString(proxy->ice_getIdentity()) + "'"};
            }

            if (_traceLevel >= 1)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << "proxy already in routing table:\n" << proxy;
            }

            _queue.erase(entry.pos);
            auto q = _queue.insert(_queue.end(), p);
            entry.pos = q;
        }

        while (static_cast<int>(_map.size()) > _maxSize)
        {
            p = _queue.front();

            if (_traceLevel >= 2)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << "evicting proxy from routing table:\n" << p->second.proxy;
            }

            evictedProxies.emplace_back(p->second.proxy);

            _map.erase(p);
            _queue.pop_front();
        }
    }

    if (_observer)
    {
        _observer->routingTableSize(static_cast<int>(_map.size()) - static_cast<int>(sz));
    }

    return evictedProxies;
}

optional<ObjectPrx>
Glacier2::RoutingTable::get(const Identity& ident)
{
    lock_guard<mutex> lock(_mutex);

    auto p = _map.find(ident);

    if (p == _map.end())
    {
        return nullopt;
    }
    else
    {
        auto& entry = p->second;
        _queue.erase(entry.pos);
        auto q = _queue.insert(_queue.end(), p);
        entry.pos = q;

        return entry.proxy;
    }
}
