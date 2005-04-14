// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <SessionFactoryI.h>

using namespace std;
using namespace Demo;

ReapThread::ReapThread(const SessionFactoryIPtr& Factory, const IceUtil::Time& timeout) :
    _destroy(false),
    _timeout(timeout),
    _factory(Factory)
{
}

ReapThread::~ReapThread()
{
    cout << "~ReapThread" << endl;
}

void
ReapThread::run()
{
    Lock sync(*this);
    while(!_destroy)
    {
	timedWait(_timeout);
	if(!_destroy)
	{
	    assert(_factory);
	    _factory->reap();
	}
    }
}

void
ReapThread::destroy()
{
    Lock sync(*this);
    _destroy = true;
    notify();
    // Drop the cyclic reference count.
    _factory = 0;
}

SessionFactoryI::SessionFactoryI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter),
    _timeout(IceUtil::Time::seconds(10)),
    _reapThread(new ReapThread(this, _timeout))
{
    _reapThread->start();
}

SessionFactoryI::~SessionFactoryI()
{
    cout << "~SessionFactoryI" << endl;
}

SessionPrx
SessionFactoryI::create(const Ice::Current& c)
{
    Lock sync(*this);

    SessionIPtr session = new SessionI(_adapter, _timeout);
    SessionPrx proxy = SessionPrx::uncheckedCast(_adapter->addWithUUID(session));
    _sessions.push_back(make_pair(session, proxy->ice_getIdentity()));
    return proxy;
}

void
SessionFactoryI::shutdown(const ::Ice::Current& c)
{
    Lock sync(*this);

    cout << "Shutting down..." << endl;
    c.adapter->getCommunicator()->shutdown();
}

void
SessionFactoryI::reap()
{
    Lock sync(*this);

    list<pair<SessionIPtr, Ice::Identity> >::iterator p = _sessions.begin();
    while(p != _sessions.end())
    {
	if(p->first->destroyed())
	{
	    p->first->destroyCallback();
    	    _adapter->remove(p->second);
    	    p = _sessions.erase(p);
	}
	else
	{
	    ++p;
	}
    }
}

void
SessionFactoryI::destroy()
{
    Lock sync(*this);

    _reapThread->destroy();
    _reapThread->getThreadControl().join();
    _reapThread = 0;

    //
    // XXX: There is an issue. This is called post after the
    // communicator->waitForShutdown() has been called. Then it
    // attempts to unregister each of the objects from the OA. This
    // causes an ObjectAdapterDeactivatedException. I think you should
    // be permitted to unregister objects even if the OA has been
    // deactivated. Note the documentation in the slice doesn't say
    // that you cannot call remove after the OA has been deactivated.
    //
    // Either Ice must be fixed, or destroy() should be called from
    // shutdown() (which isn't nice since it means I need a _destroy
    // flag, plus other things since other calls could be in
    // progress), or I must catch ObjectAdapaterDeactivateException,
    // or destroy() shouldn't call destroyCallback() on the sessions
    // (which I also don't think is very nice).
    //
/*
    for(list<pair<SessionIPtr, Ice::Identity> >::const_iterator p = _sessions.begin();
	p != _sessions.end();
	++p)
    {
	p->first->destroyCallback();
	_adapter->remove(p->second);
    }
    _sessions.clear();
*/
}
