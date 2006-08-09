// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FLUSHER_H
#define FLUSHER_H

#include <IceUtil/Handle.h>
#include <IceUtil/Shared.h>
#include <IceUtil/Thread.h>
#include <Ice/CommunicatorF.h>
#include <list>

namespace IceStorm
{

//
// Forward declarations.
//
class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Flushable;
typedef IceUtil::Handle<Flushable> FlushablePtr;
typedef std::list<FlushablePtr> FlushableList;

//
// Flusher Thread
//
class FlusherThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    FlusherThread(const Ice::CommunicatorPtr&, const TraceLevelsPtr&);
    ~FlusherThread();

    virtual void run();
    void destroy();
    void add(const FlushablePtr&);
    void remove(const FlushablePtr&);
private:

    void flushAll();
    long calcTimeout();

    Ice::CommunicatorPtr _communicator;
    TraceLevelsPtr _traceLevels;
    FlushableList _subscribers;
    bool _destroy;
    long _flushTime;
};

typedef IceUtil::Handle<FlusherThread> FlusherThreadPtr;

//
// Responsible for flushing Flushable objects at regular intervals.
//
class Flusher : public IceUtil::Shared
{
public:

    Flusher(const Ice::CommunicatorPtr&, const TraceLevelsPtr&);
    ~Flusher();

    void add(const FlushablePtr&);
    void remove(const FlushablePtr&);
    void stopFlushing();

private:

    FlusherThreadPtr _thread;
};

typedef IceUtil::Handle<Flusher> FlusherPtr;

} // End namespace IceStorm

#endif
