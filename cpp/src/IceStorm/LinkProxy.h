// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef LINK_PROXY_H
#define LINK_PROXY_H

#include <IceStorm/IceStormInternal.h>
#include <IceStorm/QueuedProxy.h>

namespace IceStorm
{

//
// Forward declarations.
//
class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

//
// LinkProxy subclasses QueuedProxy for topic links.
//
class LinkProxy : public QueuedProxy
{
public:

    LinkProxy(const TraceLevelsPtr&, const std::string&, const TopicLinkPrx&);

    virtual Ice::ObjectPrx proxy() const;

protected:

    virtual void deliver(const std::vector<EventPtr>&);

private:

    const TraceLevelsPtr _traceLevels;
    const std::string _id;
    TopicLinkPrx _obj;
};

}

#endif
