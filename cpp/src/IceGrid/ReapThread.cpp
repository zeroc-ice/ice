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

ReapThread::ReapThread(int wakeInterval) :
    _wakeInterval(IceUtil::Time::seconds(wakeInterval)),
    _terminated(false)
{
}

void
ReapThread::run()
{
    vector<ReapableItem> reap;
    while(true)
    {
	{
	    Lock sync(*this);
	    timedWait(_wakeInterval);
	    
	    if(_terminated)
	    {
		break;
	    }

	    list<ReapableItem>::iterator p = _sessions.begin();
	    while(p != _sessions.end())
	    {
		try
		{
		    if((IceUtil::Time::now() - p->item->timestamp()) > p->timeout)
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

	for(vector<ReapableItem>::const_iterator p = reap.begin(); p != reap.end(); ++p)
	{
	    p->item->destroy(false);
	}
	reap.clear();
    }
}

void
ReapThread::terminate()
{
    list<ReapableItem> reap;
    {
	Lock sync(*this);
	if(_terminated)
	{
	    return;
	}
	_terminated = true;
	notify();
	reap.swap(_sessions);
    }

    for(list<ReapableItem>::const_iterator p = reap.begin(); p != reap.end(); ++p)
    {
	p->item->destroy(true);
    }
}

void
ReapThread::add(const ReapablePtr& reapable, int timeout)
{
    Lock sync(*this);
    if(_terminated)
    {
	return;
    }

    ReapableItem item;
    item.item = reapable;
    item.timeout = IceUtil::Time::seconds(timeout);
    _sessions.push_back(item);
}

