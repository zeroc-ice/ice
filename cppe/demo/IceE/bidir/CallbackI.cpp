// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <CallbackI.h>

using namespace std;
using namespace Ice;
using namespace Demo;

CallbackSenderI::CallbackSenderI() :
    _destroy(false),
    _num(0),
    _callbackSenderThread(new CallbackSenderThread(this))
{
}

void
CallbackSenderI::destroy()
{
    Ice::ThreadPtr callbackSenderThread;

    {
	Ice::Monitor<Ice::Mutex>::Lock lock(*this);
	
	printf("destroying callback sender\n");
	_destroy = true;
	
	notify();

	callbackSenderThread = _callbackSenderThread;
	_callbackSenderThread = 0; // Resolve cyclic dependency.
    }

    callbackSenderThread->getThreadControl().join();
}

void
CallbackSenderI::addClient(const Identity& ident, const Current& current)
{
    Ice::Monitor<Ice::Mutex>::Lock lock(*this);

    printf("adding client `%s'\n", identityToString(ident).c_str());

    CallbackReceiverPrx client = CallbackReceiverPrx::uncheckedCast(current.con->createProxy(ident));
    _clients.insert(client);
}

void
CallbackSenderI::start()
{
    _callbackSenderThread->start();
}

void
CallbackSenderI::run()
{
    Ice::Monitor<Ice::Mutex>::Lock lock(*this);

    while(!_destroy)
    {
	timedWait(Ice::Time::seconds(2));

	if(!_destroy && !_clients.empty())
	{
	    ++_num;
	    
	    set<CallbackReceiverPrx>::iterator p = _clients.begin();
	    while(p != _clients.end())
	    {
		try
		{
		    (*p)->callback(_num);
		    ++p;
		}
		catch(const Exception& ex)
		{
		    fprintf(stderr, "removing client `%s':\n%s\n", identityToString((*p)->ice_getIdentity()).c_str(),
		    	    ex.toString().c_str());
		    _clients.erase(p++);
		}
	    }
	}
    }
}
