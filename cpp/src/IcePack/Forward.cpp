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
#include <IcePack/Forward.h>
#include <IcePack/Admin.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::Forward::Forward(const CommunicatorPtr& communicator, const AdminPtr& admin) :
    _communicator(communicator),
    _admin(admin)
{
#ifndef _WIN32
    _activator = new Activator(_communicator);
    _activator->start();

    _waitTime = _communicator->getProperties()->getPropertyAsIntWithDefault("IcePack.Activator.WaitTime", 10);
    if(_waitTime < 0)
    {
	_waitTime = 0;
    }
#endif
}

IcePack::Forward::~Forward()
{
}

ObjectPtr
IcePack::Forward::locate(const ObjectAdapterPtr& adapter, const Current& current, LocalObjectPtr&)
{
    //
    // Look up the server description
    //
    ServerDescription desc = _admin->find(current.id, current);

    //
    // If we didn't find a server description, we return null, meaning
    // that the client will get an "object not exist" exception.
    //
    if(!desc.obj)
    {
	return 0;
    }

#ifndef _WIN32

    assert(_activator);

    //
    // We only try to activate if we have a path for the server
    //
    if(!desc.path.empty())
    {
	try
	{
	    bool doSleep = false;
	    int count = 0;
	    while(true)
	    {
		try
		{
		    //
		    // Activate the server. If the server is already
		    // running, this operation does nothing.
		    //
		    if(_activator->activate(desc))
		    {
			//
			// If we just activated the server, we sleep
			// below, to give the server some time to
			// start up.
			//
			doSleep = true;
		    }
		    
		    if(doSleep)
		    {
			sleep(1);
		    }
		    
		    //
		    // Try to ping the server, to make sure that it is
		    // really running. Note that even if activate()
		    // above returns false, i.e., if activate()
		    // indicates that the server was already running,
		    // it's still possible that the server shut down
		    // in the meantime, for example, because of a
		    // server timeout, a crash, or an explicit
		    // shutdown method.
		    //
		    desc.obj->ice_ping();

		    //
		    // Everything ok, the server is now up and
		    // running. The ping above also has the effect
		    // that the server idle timeout (if set) has been
		    // reset. If we wouldn't ping, there would be a
		    // higher chance that the server shuts down
		    // because of an idle timeout while we send back
		    // the location forward to the client.
		    //
		    break;
		}
		catch(const SocketException&)
		{
		    //
		    // Ooops, we got a socket exception while trying
		    // to ping the server. Let's set the doSleep flag
		    // to give the server more time before we try
		    // again.
		    //
		    if(++count >= _waitTime)
		    {
			throw;
		    }
		    
		    doSleep = true;
		}
	    }
	}
	catch(const Exception& ex)
	{
	    //
	    // If we get an exception, all we an do is to log it and
	    // to send a location forward to the client, which will
	    // then get a similar exception.
	    //
	    Error out(_communicator->getLogger());
	    out << "exception during server activation:\n" << ex;
	}
    }

#endif

    throw LocationForward(desc.obj);
}

void
IcePack::Forward::finished(const ObjectAdapterPtr&, const Current&, const ObjectPtr&, const LocalObjectPtr&)
{
    // Nothing to do
}

void
IcePack::Forward::deactivate()
{
#ifndef _WIN32
    _activator->destroy();
    _activator->getThreadControl().join();
    _activator = 0;
#endif
}
