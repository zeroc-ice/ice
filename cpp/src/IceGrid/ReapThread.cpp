// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Glacier2/Session.h>
#include <IceGrid/ReapThread.h>

using namespace std;
using namespace IceGrid;

ReapThread::ReapThread(int timeout) :
    _timeout(IceUtil::Time::seconds(timeout)),
    _terminated(false)
{
}

void
ReapThread::run()
{
    Lock sync(*this);

    while(!_terminated)
    {
	list<pair<SessionIPtr, Glacier2::SessionPrx> >::iterator p = _sessions.begin();
	while(p != _sessions.end())
	{
	    try
	    {
		if((IceUtil::Time::now() - p->first->timestamp()) > _timeout)
		{
		    try
		    {
			p->second->destroy();
		    }
		    catch(const Ice::LocalException&)
		    {
		    }
		    p = _sessions.erase(p);
		}
		else
		{
		    ++p;
		}
	    }
	    catch(const Ice::ObjectNotExistException&)
	    {
		p = _sessions.erase(p);
	    }
	}

	timedWait(_timeout);
    }
}

void
ReapThread::terminate()
{
    Lock sync(*this);

    _terminated = true;
    notify();

    for(list<pair<SessionIPtr, Glacier2::SessionPrx> >::const_iterator p = _sessions.begin(); p != _sessions.end(); ++p)
    {
	try
	{
	    p->second->destroy();
	}
	catch(const Ice::Exception&)
	{
	    // Ignore.
	}
    }

    _sessions.clear();
}

void
ReapThread::add(const Glacier2::SessionPrx& proxy, const SessionIPtr& session)
{
    Lock sync(*this);
    _sessions.push_back(make_pair(session, proxy));
}

