// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/ServerAdapterI.h>
#include <IcePack/ServerFactory.h>
#include <IcePack/TraceLevels.h>

using namespace std;
using namespace IcePack;

IcePack::ServerAdapterI::ServerAdapterI(const ServerFactoryPtr& factory, const TraceLevelsPtr& traceLevels, 
					Ice::Int waitTime) :
    _factory(factory),
    _traceLevels(traceLevels),
    _waitTime(waitTime)
{
}

IcePack::ServerAdapterI::~ServerAdapterI()
{
}

Ice::ObjectPrx
IcePack::ServerAdapterI::getDirectProxy(bool activate, const Ice::Current& current)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    if(!_proxy && activate)
    {
	//
	// Try to start the server and wait for the adapter proxy to
	// be changed.
	//

	if(_traceLevels->adapter > 2)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	    out << "waiting for activation of server adapter `" << name << "'";
	}
	
	try
	{
	    if(server->start(OnDemand))
	    {
		_notified = false;

		while(!_notified)
		{
		    bool notify = timedWait(IceUtil::Time::seconds(_waitTime));
		    if(!notify)
		    {
			if(_traceLevels->adapter > 1)
			{
			    Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
			    out << "server adapter `" << name << "' activation timed out";
			}
		    }
		}
	    }
	    else
	    {
		if(_traceLevels->adapter > 1)
		{
		    Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
		    out << "server adapter `" << name << "' activation failed, couldn't start the server";
		}
	    }
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    //
	    // The server associated to this adapter doesn't exist
	    // anymore. Somehow the database is inconsistent if this
	    // happens. The best thing to do is to destroy the adapter
	    // and throw an ObjectNotExist exception.
	    //
	    destroy(current);
	    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}
    }
    return _proxy;
}

void
IcePack::ServerAdapterI::setDirectProxy(const Ice::ObjectPrx& prx, const Ice::Current&)
{
    ::IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    //
    // If the adapter proxy is not null the given proxy can only be
    // null. We don't allow to overide an existing proxy by another
    // non null proxy.
    //
    if(prx && _proxy)
    {
	throw AdapterActiveException();
    }

    _proxy = prx;
    _notified = true;

    if(_traceLevels->adapter > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	out << "server adapter `" << name << "' " << (_proxy ? "activated" : "deactivated");
    }
    
    notifyAll();
}

void
IcePack::ServerAdapterI::destroy(const Ice::Current& current)
{
    _factory->destroy(this, current.id);
}
