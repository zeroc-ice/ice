// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <AdminI.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

AdminI::AdminI(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
AdminI::add(const ServerDescriptionPtr& p)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (p && p->object)
    {
	_map[p->object] = p;
    }
}

void
AdminI::remove(const ObjectPrx& p)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (p)
    {
	_map.erase(p);
    }
}

ServerDescriptionPtr
AdminI::find(const ObjectPrx& p)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (p)
    {
	map<ObjectPrx, ServerDescriptionPtr>::iterator q = _map.find(p);
	if (q != _map.end())
	{
	    return q->second;
	}
	else
	{
	    return 0;
	}
    }
    else
    {
	return 0;
    }
}

void
AdminI::shutdown()
{
    _communicator->shutdown();
}
