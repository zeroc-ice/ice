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

#include <Ice/LoggerF.h>

#include <IceStorm/FlusherF.h>
#include <IceStorm/SubscriberF.h>
#include <IceStorm/TraceLevelsF.h>

namespace IceStorm
{

class FlusherThread;
typedef JTCHandleT<FlusherThread> FlusherThreadHandle;    

class Flusher : public IceUtil::Shared
{
public:

    Flusher(const Ice::CommunicatorPtr&, const TraceLevelsPtr&);
    ~Flusher();

    void add(const SubscriberPtr&);
    void remove(const SubscriberPtr&);
    void stopFlushing();

private:

    FlusherThreadHandle _thread;
};

} // End namespace IceStorm

#endif
