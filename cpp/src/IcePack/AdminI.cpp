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
	_map[p->object->_getIdentity()] = p;
    }
}

void
IcePack::AdminI::remove(const string& identity)
{
    JTCSyncT<JTCMutex> sync(*this);
    _map.erase(identity);
}

ServerDescriptionPtr
IcePack::AdminI::find(const string& identity)
{
    JTCSyncT<JTCMutex> sync(*this);

    map<string, ServerDescriptionPtr>::iterator p = _map.find(identity);
    if (p != _map.end())
    {
	return p->second;
    }
    else
    {
	return 0;
    }
}

ServerDescriptions
IcePack::AdminI::getAll()
{
    ServerDescriptions result;
    result.reserve(_map.size());
    for (map<string, ServerDescriptionPtr>::iterator p = _map.begin(); p != _map.end(); ++p)
    {
	result.push_back(p->second);
    }
    return result;
}

void
IcePack::AdminI::shutdown()
{
    _communicator->shutdown();
}
