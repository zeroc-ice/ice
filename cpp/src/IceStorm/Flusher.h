// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
typedef JTCHandleT<FlusherThread> FlusherThreadHandle;    

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

    FlusherThreadHandle _thread;
};

typedef IceUtil::Handle<Flusher> FlusherPtr;

} // End namespace IceStorm

#endif
