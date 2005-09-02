// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/LinkProxy.h>

using namespace std;

IceStorm::LinkProxy::LinkProxy(const TopicLinkPrx& obj) :
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
    _obj->forward(events);
}
