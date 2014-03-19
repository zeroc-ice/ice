// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef EVICTOR_BASE_H
#define EVICTOR_BASE_H

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <map>
#include <list>

class EvictorBase : public Ice::ServantLocator
{
public:

    EvictorBase(int size = 1000);

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

protected:

    virtual Ice::ObjectPtr add(const Ice::Current&, Ice::LocalObjectPtr&) = 0;
    virtual void evict(const Ice::ObjectPtr&, const Ice::LocalObjectPtr&) = 0;

private:

    struct EvictorEntry;
    typedef IceUtil::Handle<EvictorEntry> EvictorEntryPtr;

    typedef std::map<Ice::Identity, EvictorEntryPtr> EvictorMap;
    typedef std::list<EvictorMap::iterator> EvictorQueue;

    struct EvictorEntry : public Ice::LocalObject
    {
        Ice::ObjectPtr servant;
        Ice::LocalObjectPtr userCookie;
        EvictorQueue::iterator queuePos;
        int useCount;
    };

    EvictorMap _map;
    EvictorQueue _queue;
    Ice::Int _size;

    IceUtil::Mutex _mutex;

    void evictServants();
};
typedef IceUtil::Handle<EvictorBase> EvictorBasePtr;

#endif
