// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/RoutingTable.h>
#include <Ice/Proxy.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(RoutingTable* p) { p->__incRef(); }
void IceInternal::decRef(RoutingTable* p) { p->__decRef(); }

IceInternal::RoutingTable::RoutingTable() :
    _tableHint(_table.end())
{
}

bool
IceInternal::RoutingTable::add(const ObjectPrx& prx)
{
    if (!prx)
    {
	return false;
    }

    ObjectPrx proxy = prx->ice_default(); // We insert the proxy in it's default form into the routing table.

    IceUtil::Mutex::Lock sync(*this);

    map<Identity, ObjectPrx>::iterator p = _table.end();
    
    if (_tableHint != _table.end())
    {
	if (_tableHint->first == proxy->ice_getIdentity())
	{
	    p = _tableHint;
	}
    }
    
    if (p == _table.end())
    {
	p = _table.find(proxy->ice_getIdentity());
    }

    if (p == _table.end())
    {
	_tableHint = _table.insert(_tableHint, make_pair(proxy->ice_getIdentity(), proxy));
	return true;
    }
    else
    {
	return false;
    }
}

ObjectPrx
IceInternal::RoutingTable::get(const Identity& ident)
{
    if (ident.name.empty())
    {
	return 0;
    }

    IceUtil::Mutex::Lock sync(*this);

    map<Identity, ObjectPrx>::iterator p = _table.end();
    
    if (_tableHint != _table.end())
    {
	if (_tableHint->first == ident)
	{
	    p = _tableHint;
	}
    }
    
    if (p == _table.end())
    {
	p = _table.find(ident);
    }

    if (p == _table.end())
    {
	return 0;
    }
    else
    {
	_tableHint = p;
	return p->second;
    }
}
