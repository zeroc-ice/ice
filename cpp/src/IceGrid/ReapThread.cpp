// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
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
    vector<ReapablePtr> reap;
    while(true)
    {
	{
	    Lock sync(*this);
	    timedWait(_timeout);
	    
	    if(_terminated)
	    {
		break;
	    }

	    list<ReapablePtr>::iterator p = _sessions.begin();
	    while(p != _sessions.end())
	    {
		try
		{
		    if((IceUtil::Time::now() - (*p)->timestamp()) > _timeout)
		    {
			reap.push_back(*p);
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
	}

	for(vector<ReapablePtr>::const_iterator p = reap.begin(); p != reap.end(); ++p)
	{
	    (*p)->destroy(false);
	}
	reap.clear();
    }
}

void
ReapThread::terminate()
{
    list<ReapablePtr> reap;
    {
	Lock sync(*this);
	_terminated = true;
	notify();
	reap.swap(_sessions);
    }

    for(list<ReapablePtr>::const_iterator p = reap.begin(); p != reap.end(); ++p)
    {
	(*p)->destroy(true);
    }
}

void
ReapThread::add(const ReapablePtr& reapable)
{
    Lock sync(*this);
    if(_terminated)
    {
	return;
    }
    _sessions.push_back(reapable);
}

