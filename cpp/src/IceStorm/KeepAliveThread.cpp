// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/KeepAliveThread.h>
#include <IceStorm/Instance.h>
#include <IceStorm/TraceLevels.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Communicator.h>
#include <Ice/Properties.h>

using namespace std;
using namespace IceStorm;

KeepAliveThread::KeepAliveThread(const InstancePtr& instance) :
    _instance(instance),
    _timeout(IceUtil::Time::seconds(max(instance->properties()->getPropertyAsIntWithDefault(
					    "IceStorm.KeepAliveTimeout", 60), 1))), // min 1s.
    _publish(false),
    _destroy(false)
{
    start();
}

KeepAliveThread::~KeepAliveThread()
{
}

void
KeepAliveThread::add(const TopicUpstreamLinkPrx& upstream)
{
    Lock sync(*this);
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->keepAlive > 0)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->keepAliveCat);
	out << "add " << _instance->communicator()->identityToString(upstream->ice_getIdentity());
    }
    _upstream.push_back(upstream);
    //
    // If we've added the first item then we need to wake the ping
    // thread since it sleeps indefinitely if there are no upstream
    // topics to notify.
    //
    if(_upstream.size() == 1)
    {
	notify();
    }
}

void
KeepAliveThread::remove(const TopicUpstreamLinkPrx& upstream)
{
    Lock sync(*this);
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->keepAlive > 0)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->keepAliveCat);
	out << "remove " << _instance->communicator()->identityToString(upstream->ice_getIdentity());
    }
    list<TopicUpstreamLinkPrx>::iterator p = find(_upstream.begin(), _upstream.end(), upstream);
    if(p != _upstream.end())
    {
	_upstream.erase(p);
    }
}

void
KeepAliveThread::startPinging()
{
    Lock sync(*this);
    _publish = true;
    notify();
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
	    TraceLevelsPtr traceLevels = _instance->traceLevels();
	    if(traceLevels->keepAlive > 0)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->keepAliveCat);
		out << "filter " << _instance->communicator()->identityToString((*p)->ice_getIdentity());
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
    while(true)
    {
	list<TopicUpstreamLinkPrx> upstream;
	while(true)
	{
	    Lock sync(*this);
	    if(!_destroy)
	    {
		if(!_publish || _upstream.size() == 0)
		{
		    wait();
		}
		else
		{
		    timedWait(_timeout);
		}
	    }
	    if(_destroy)
	    {
		return;
	    }
	    if(!_publish)
	    {
		continue;
	    }
	    upstream = _upstream;
	    break;
	}

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
		TraceLevelsPtr traceLevels = _instance->traceLevels();
		if(traceLevels->keepAlive > 1)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->keepAliveCat);
		    out << "unreachable " << _instance->communicator()->identityToString((*p)->ice_getIdentity());
		}
		// Ignore
	    }
	}
    }
}

void
KeepAliveThread::failed(const TopicUpstreamLinkPrx& upstream)
{
    Lock sync(*this);
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->keepAlive > 1)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->keepAliveCat);
	out << "failed " << _instance->communicator()->identityToString(upstream->ice_getIdentity());
    }
    list<TopicUpstreamLinkPrx>::iterator p = find(_upstream.begin(), _upstream.end(), upstream);
    if(p != _upstream.end())
    {
	_upstream.erase(p);
    }
    _failed.push_back(upstream);
}
