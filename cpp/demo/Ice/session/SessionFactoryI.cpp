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

    for(list<pair<SessionIPtr, Ice::Identity> >::const_iterator p = _sessions.begin();
	p != _sessions.end();
	++p)
    {
	p->first->destroyCallback();

    	// When the session factory is destroyed the OA is deactivated
    	// and all servants have been // removed so calling remove on
    	// the OA is not necessary.
    }
    _sessions.clear();
}
