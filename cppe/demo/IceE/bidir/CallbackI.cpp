// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <CallbackI.h>

using namespace std;
using namespace Demo;

CallbackSenderI::CallbackSenderI(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator),
    _destroy(false),
    _num(0),
    _callbackSenderThread(new CallbackSenderThread(this))
{
}

void
CallbackSenderI::destroy()
{
    IceUtil::ThreadPtr callbackSenderThread;

    {
	Lock lock(*this);
	
	printf("destroying callback sender\n");
	_destroy = true;
	
	notify();

	callbackSenderThread = _callbackSenderThread;
	_callbackSenderThread = 0; // Resolve cyclic dependency.
    }

    callbackSenderThread->getThreadControl().join();
}

void
CallbackSenderI::addClient(const Ice::Identity& ident, const Ice::Current& current)
{
    Lock lock(*this);

    printf("adding client `%s'\n", _communicator->identityToString(ident).c_str());

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
    Lock lock(*this);

    while(!_destroy)
    {
	timedWait(IceUtil::Time::seconds(2));

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
		catch(const Ice::Exception& ex)
		{
		    fprintf(stderr, "removing client `%s':\n%s\n",
			    _communicator->identityToString((*p)->ice_getIdentity()).c_str(),
		    	    ex.toString().c_str());
		    _clients.erase(p++);
		}
	    }
	}
    }
}
