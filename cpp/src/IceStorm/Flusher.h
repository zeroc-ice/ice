// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FLUSHER_H
#define FLUSHER_H

#include <IceUtil/Handle.h>
#include <IceUtil/Shared.h>
#include <Ice/CommunicatorF.h>

namespace IceStorm
{

//
// Forward declarations.
//
class FlusherThread;
typedef IceUtil::Handle<FlusherThread> FlusherThreadPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Flushable;
typedef IceUtil::Handle<Flushable> FlushablePtr;

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
