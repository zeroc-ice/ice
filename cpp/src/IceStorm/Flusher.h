// **********************************************************************
//
// Copyright (c) 2001
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
