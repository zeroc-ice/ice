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
IcePack::AdminI::add(const ServerDescription& desc)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (desc.object)
    {
	_serverDescriptions[desc.object->_getIdentity()] = desc;
    }
}

void
IcePack::AdminI::remove(const string& identity)
{
    JTCSyncT<JTCMutex> sync(*this);
    _serverDescriptions.erase(identity);
}

ServerDescription
IcePack::AdminI::find(const string& identity)
{
    JTCSyncT<JTCMutex> sync(*this);

    ServerDescriptions::iterator p = _serverDescriptions.find(identity);
    if (p != _serverDescriptions.end())
    {
	return p->second;
    }
    else
    {
	return ServerDescription();
    }
}

ServerDescriptions
IcePack::AdminI::getAll()
{
    return _serverDescriptions;
}

void
IcePack::AdminI::shutdown()
{
    _communicator->shutdown();
}
