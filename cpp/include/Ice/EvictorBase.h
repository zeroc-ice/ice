// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_EVICTORBASE_H
#define ICE_EVICTORBASE_H

#include <Ice/Evictor.h>
#include <Ice/CommunicatorF.h>
#include <IceUtil/Mutex.h>
#include <list>

namespace Ice
{

class ICE_API EvictorBase : virtual public Ice::Evictor
{
public:

    EvictorBase();

    virtual void setSize(Ice::Int);
    virtual Ice::Int getSize() const;

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

    virtual Ice::ObjectPtr instantiate(const Ice::Current&, Ice::LocalObjectPtr&) = 0;
    virtual void evict(const Ice::Identity&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&) = 0;

private:

    void evictServants();
    void initialize(const Ice::Current&);

    struct EvictorEntry;
    typedef IceUtil::Handle<EvictorEntry> EvictorEntryPtr;

    typedef std::list<Ice::Identity> EvictorQueue;
    typedef std::map<Ice::Identity, EvictorEntryPtr> EvictorMap;

    struct EvictorEntry : public Ice::LocalObject
    {
	int useCount;
	EvictorQueue::iterator pos;
	Ice::ObjectPtr servant;
	Ice::LocalObjectPtr userCookie;
    };

    struct EvictorCookie : public Ice::LocalObject
    {
	EvictorEntryPtr entry;
    };
    typedef IceUtil::Handle<EvictorCookie> EvictorCookiePtr;

    EvictorQueue _queue;
    EvictorMap _map;
    Ice::Int _size;
    bool _initialized;
    size_t _hits;
    size_t _misses;
    size_t _evictions;
    int _traceLevel;
    Ice::CommunicatorPtr _communicator;

    IceUtil::Mutex _mutex;

    enum { defaultSize = 1000 };

};
typedef IceUtil::Handle<Ice::EvictorBase> EvictorBasePtr;

}

#endif
