// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>

#include <IceStorm/LinkProxy.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Event.h>

using namespace std;

IceStorm::LinkProxy::LinkProxy(const TraceLevelsPtr& traceLevels, const string& id, const TopicLinkPrx& obj) :
    _traceLevels(traceLevels),
    _id(id),
    _obj(obj)
{
}

Ice::ObjectPrx
IceStorm::LinkProxy::proxy() const
{
    return _obj;
}

void
IceStorm::LinkProxy::deliver(const vector<EventPtr>& v)
{
    //
    // TODO: It would be nice to avoid all this copying. To do this we
    // could change EventData from a struct to a class and rename to
    // Event, and then remove EventPtr. However, this means EventData
    // would have some data that doesn't need to be passed over the
    // wire (forward/cost), and the marshaling of the data would be
    // marginally slower.
    //
    vector<EventData> events;
    for(vector<EventPtr>::const_iterator p = v.begin(); p != v.end(); ++p)
    {
	EventData e;
	e.op = (*p)->op;
	e.mode = (*p)->mode;
	e.data = (*p)->data;
	e.context = (*p)->context;
	events.push_back(e);
    }

    try
    {
	_obj->forward(events);
    }
    catch(const Ice::ObjectNotExistException&)
    {
	//
	// ObjectNotExist causes the link to be removed.
	//
	throw;
    }
    catch(const Ice::LocalException& e)
    {
	if(_traceLevels->subscriber > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	    out << _id << ": link topic publish failed: " << e;
	}
    }
}
