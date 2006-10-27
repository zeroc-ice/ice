// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/KeepAliveThread.h>
#include <IceStorm/TraceLevels.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Communicator.h>

using namespace std;
using namespace IceStorm;

KeepAliveThread::KeepAliveThread(const Ice::CommunicatorPtr& communicator, const TraceLevelsPtr& traceLevels,
				 const IceUtil::Time& timeout) :
    _communicator(communicator),
    _traceLevels(traceLevels),
    _timeout(timeout),
    _destroy(false)
{
}

KeepAliveThread::~KeepAliveThread()
{
}

void
KeepAliveThread::add(const TopicUpstreamLinkPrx& upstream)
{
    Lock sync(*this);
    if(_traceLevels->keepAlive > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->keepAliveCat);
	out << "add " << _communicator->identityToString(upstream->ice_getIdentity());
    }
    _upstream.push_back(upstream);
}

void
KeepAliveThread::remove(const TopicUpstreamLinkPrx& upstream)
{
    Lock sync(*this);
    if(_traceLevels->keepAlive > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->keepAliveCat);
	out << "remove " << _communicator->identityToString(upstream->ice_getIdentity());
    }
    list<TopicUpstreamLinkPrx>::iterator p = find(_upstream.begin(), _upstream.end(), upstream);
    if(p != _upstream.end())
    {
	_upstream.erase(p);
    }
}

void
KeepAliveThread::destroy()
{
    Lock sync(*this);
    _destroy = true;
    notify();
}

bool
KeepAliveThread::filter(IceStorm::TopicUpstreamLinkPrxSeq& upstream)
{
    Lock sync(*this);
    bool changed = false;
    IceStorm::TopicUpstreamLinkPrxSeq::iterator p = upstream.begin();
    while(p != upstream.end())
    {
	list<TopicUpstreamLinkPrx>::iterator q = find(_failed.begin(), _failed.end(), *p);
	if(q != _failed.end())
	{
	    if(_traceLevels->keepAlive > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->keepAliveCat);
		out << "filter " << _communicator->identityToString((*p)->ice_getIdentity());
	    }
	    _failed.erase(q);
	    p = upstream.erase(p);
	    changed = true;
	}
	else
	{
	    ++p;
	}
    }
    return changed;
}

void
KeepAliveThread::run()
{
    list<TopicUpstreamLinkPrx> upstream;
    {
	Lock sync(*this);
	upstream = _upstream;
    }

    //
    // First all upstream links are notified. Then we wait. It is done
    // in this order so that any upstream links are notified
    // immediately upon startup of the service.
    //
    for(;;)
    {
	for(list<TopicUpstreamLinkPrx>::const_iterator p = upstream.begin(); p != upstream.end(); ++p)
	{
	    try
	    {
		(*p)->keepAlive();
	    }
	    catch(const Ice::ObjectNotExistException&)
	    {
		failed(*p);
	    }
	    catch(const Ice::Exception&)
	    {
		if(_traceLevels->keepAlive > 1)
		{
		    Ice::Trace out(_traceLevels->logger, _traceLevels->keepAliveCat);
		    out << "unreachable " << _communicator->identityToString((*p)->ice_getIdentity());
		}
		// Ignore
	    }
	}

	{
	    Lock sync(*this);
	    timedWait(_timeout);
	    if(_destroy)
	    {
		return;
	    }
	    upstream = _upstream;
	}
    }
}

void
KeepAliveThread::failed(const TopicUpstreamLinkPrx& upstream)
{
    Lock sync(*this);
    if(_traceLevels->keepAlive > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->keepAliveCat);
	out << "failed " << _communicator->identityToString(upstream->ice_getIdentity());
    }
    list<TopicUpstreamLinkPrx>::iterator p = find(_upstream.begin(), _upstream.end(), upstream);
    if(p != _upstream.end())
    {
	_upstream.erase(p);
    }
    _failed.push_back(upstream);
}
