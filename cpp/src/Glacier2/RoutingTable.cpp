// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/RoutingTable.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::RoutingTable::RoutingTable(const CommunicatorPtr& communicator) :
    _communicator(communicator),
    _traceLevel(_communicator->getProperties()->getPropertyAsInt("Glacier2.Trace.RoutingTable")),
    _maxSize(_communicator->getProperties()->getPropertyAsIntWithDefault("Glacier2.RoutingTable.MaxSize", 1000)),
    _verifier(communicator, "")
{
}

ObjectProxySeq
Glacier2::RoutingTable::add(const ObjectProxySeq& unfiltered, const Ice::Current& current)
{
    IceUtil::Mutex::Lock sync(*this);

    //
    // We 'pre-scan' the list, applying our validation rules. The
    // ensures that our state is not modified if this operation results
    // in a rejection.
    //
    ObjectProxySeq proxies; 
    ObjectProxySeq::const_iterator prx;
    for(prx = unfiltered.begin(); prx != unfiltered.end(); ++prx)
    {
        if(!*prx) // We ignore null proxies.
        {
            continue;
        }

        if(!_verifier.verify(*prx))
        {
            current.con->close(true);
            throw ObjectNotExistException(__FILE__, __LINE__);
        }
        ObjectPrx proxy = (*prx)->ice_twoway()->ice_secure(false)->ice_facet(""); // We add proxies in default form.
        proxies.push_back(proxy);
    }

    ObjectProxySeq evictedProxies;
    for(prx = proxies.begin(); prx != proxies.end(); ++prx)
    {
        ObjectPrx proxy = *prx;
        EvictorMap::iterator p = _map.find(proxy->ice_getIdentity());
        
        if(p == _map.end())
        {
            if(_traceLevel == 1 || _traceLevel >= 3)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << "adding proxy to routing table:\n" << _communicator->proxyToString(proxy);
            }
            
            EvictorEntryPtr entry = new EvictorEntry;
            p = _map.insert(_map.begin(), pair<const Identity, EvictorEntryPtr>(proxy->ice_getIdentity(), entry));
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
            
            EvictorEntryPtr entry = p->second;
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

    return evictedProxies;
}

ObjectPrx
Glacier2::RoutingTable::get(const Identity& ident)
{
    if(ident.name.empty())
    {
        return 0;
    }

    IceUtil::Mutex::Lock sync(*this);

    EvictorMap::iterator p = _map.find(ident);

    if(p == _map.end())
    {
        return 0;
    }
    else
    {
        EvictorEntryPtr entry = p->second;
        _queue.erase(entry->pos);
        EvictorQueue::iterator q = _queue.insert(_queue.end(), p);
        entry->pos = q;

        return entry->proxy;
    }
}
