// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    _maxSize(_communicator->getProperties()->getPropertyAsIntWithDefault("Glacier2.RoutingTable.MaxSize", 1000))
{
}

bool
Glacier2::RoutingTable::add(const ObjectPrx& prx)
{
    if(!prx)
    {
	return false;
    }

    //
    // We insert the proxy in its default form into the routing table.
    //
    ObjectPrx proxy = prx->ice_twoway()->ice_secure(false);

    IceUtil::Mutex::Lock sync(*this);

    EvictorMap::iterator p = _map.find(proxy->ice_getIdentity());

    if(p == _map.end())
    {
	if(_traceLevel)
	{
	    Trace out(_communicator->getLogger(), "Glacier2");
	    out << "adding proxy to routing table:\n" << _communicator->proxyToString(proxy);
	}

	EvictorEntryPtr entry = new EvictorEntry;
	p = _map.insert(_map.begin(), pair<const Identity, EvictorEntryPtr>(proxy->ice_getIdentity(), entry));
	EvictorQueue::iterator q = _queue.insert(_queue.end(), p);
	entry->proxy = proxy;
	entry->pos = q;

	return true;
    }
    else
    {
	if(_traceLevel)
	{
	    Trace out(_communicator->getLogger(), "Glacier2");
	    out << "proxy already in routing table:\n" << _communicator->proxyToString(proxy);
	}

	EvictorEntryPtr entry = p->second;
	_queue.erase(entry->pos);
	EvictorQueue::iterator q = _queue.insert(_queue.end(), p);
	entry->pos = q;

	return false;
    }
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
