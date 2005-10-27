// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

void
IceInternal::RoutingTable::clear()
{
    IceUtil::Mutex::Lock sync(*this);

    _table.clear();
    _tableHint = _table.end();
}

bool
IceInternal::RoutingTable::add(const ObjectPrx& prx)
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

    map<Identity, ObjectPrx>::iterator p = _table.end();
    
    if(_tableHint != _table.end())
    {
	if(_tableHint->first == proxy->ice_getIdentity())
	{
	    p = _tableHint;
	}
    }
    
    if(p == _table.end())
    {
	p = _table.find(proxy->ice_getIdentity());
    }

    if(p == _table.end())
    {
	_tableHint = _table.insert(_tableHint, pair<const Identity, ObjectPrx>(proxy->ice_getIdentity(), proxy));
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
    if(ident.name.empty())
    {
	return 0;
    }

    IceUtil::Mutex::Lock sync(*this);

    map<Identity, ObjectPrx>::iterator p = _table.end();
    
    if(_tableHint != _table.end())
    {
	if(_tableHint->first == ident)
	{
	    p = _tableHint;
	}
    }
    
    if(p == _table.end())
    {
	p = _table.find(ident);
    }

    if(p == _table.end())
    {
	return 0;
    }
    else
    {
	_tableHint = p;
	return p->second;
    }
}
