// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ReapThread.h>

using namespace std;
using namespace Demo;

ReapThreadPtr ReapThread::_instance;
IceUtil::StaticMutex ReapThread::_instanceMutex = ICE_STATIC_MUTEX_INITIALIZER;

ReapThreadPtr&
ReapThread::instance()
{
    IceUtil::StaticMutex::Lock sync(_instanceMutex);
    if(!_instance)
    {
	_instance = new ReapThread;
    }

    return _instance;
}

ReapThread::~ReapThread()
{
}

void
ReapThread::run()
{
    Lock sync(*this);
    while(!_terminated)
    {
	timedWait(_timeout);
	if(!_terminated)
	{
	    IceUtil::Time now = IceUtil::Time::now();
	    map<SessionPrx, SessionIPtr>::iterator p = _sessions.begin();
	    while(p != _sessions.end())
	    {
		try
		{
		    if((now - p->second->timestamp()) > _timeout)
		    {
			cout << "The session #" << Ice::identityToString(p->first->ice_getIdentity())
			     << " has timed out." << endl;
			p->first->destroy();
			map<SessionPrx, SessionIPtr>::iterator tmp = p;
			++p;
			_sessions.erase(tmp);
		    }
		    else
		    {
			++p;
		    }
    	    	}
		catch(const Ice::ObjectNotExistException&)
		{
		    map<SessionPrx, SessionIPtr>::iterator tmp = p;
		    ++p;
		    _sessions.erase(tmp);
		}
	    }
	}
    }
}

void
ReapThread::terminate()
{
    Lock sync(*this);
    _terminated = true;
    notify();

    for(map<SessionPrx, SessionIPtr>::const_iterator p = _sessions.begin();
	p != _sessions.end();
	++p)
    {
	try
	{
	    p->first->destroy();
	}
	catch(const Ice::Exception&)
	{
	    // Ignore the exception
	}
    }
    _sessions.clear();
}

void
ReapThread::add(const SessionPrx& proxy, const SessionIPtr& session)
{
    Lock sync(*this);
    _sessions.insert(make_pair(proxy, session));
}

ReapThread::ReapThread() :
    _timeout(IceUtil::Time::seconds(10)),
    _terminated(false)
{
}
