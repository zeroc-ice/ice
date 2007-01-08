// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef BATCH_FLUSHER_H
#define BATCH_FLUSHER_H

#include <IceUtil/Thread.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <Ice/ProxyF.h>
#include <list>

namespace IceStorm
{

class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

//
// Responsible for flushing Flushable objects at regular intervals.
//
class BatchFlusher : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    BatchFlusher(const InstancePtr&);
    ~BatchFlusher();

    void add(const Ice::ObjectPrx&);
    void remove(const Ice::ObjectPrx&);
    void destroy();

    void run();

private:

    const TraceLevelsPtr _traceLevels;
    const IceUtil::Time _flushTime;
    std::list<Ice::ObjectPrx> _subscribers;
    bool _destroy;
};

typedef IceUtil::Handle<BatchFlusher> BatchFlusherPtr;

} // End namespace IceStorm

#endif
