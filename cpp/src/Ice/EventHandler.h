// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_EVENT_HANDLER_H
#define ICE_EVENT_HANDLER_H

#include <IceUtil/Shared.h>
#include <Ice/EventHandlerF.h>
#include <Ice/InstanceF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/BasicStream.h>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

class EventHandler : public ::IceUtil::Shared
{
public:
    
    //
    // Return true if read() must be called before calling message().
    //
    virtual bool readable() const = 0;

    //
    // Read data via the event handler. May only be called if
    // readable() returns true.
    //
    virtual void read(BasicStream&) = 0;

    //
    // A complete message has been received.
    //
    virtual void message(BasicStream&, const ThreadPoolPtr&) = 0;

    //
    // Will be called if the event handler is finally
    // unregistered. (Calling unregister() does not unregister
    // immediately.)
    //
    virtual void finished(const ThreadPoolPtr&) = 0;

    //
    // Propagate an exception to the event handler.
    //
    virtual void exception(const ::Ice::LocalException&) = 0;

    //
    // Get a textual representation of the event handler.
    //
    virtual std::string toString() const = 0;
    
protected:
    
    EventHandler(const InstancePtr&);
    virtual ~EventHandler();

    const InstancePtr _instance;

private:

    //
    // The _stream data member is for use by ThreadPool only
    //
    BasicStream _stream;
    friend class ThreadPool;
};

}

#endif
