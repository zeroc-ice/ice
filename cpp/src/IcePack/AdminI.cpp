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
#include <IcePack/AdminI.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::AdminI::AdminI(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
IcePack::AdminI::add(const ServerDescriptionPtr& p)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (p && p->object)
    {
	_map[p->object] = p;
    }
}

void
IcePack::AdminI::remove(const ObjectPrx& p)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (p)
    {
	_map.erase(p);
    }
}

ServerDescriptionPtr
IcePack::AdminI::find(const ObjectPrx& p)
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
IcePack::AdminI::shutdown()
{
    _communicator->shutdown();
}
