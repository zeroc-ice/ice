// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

class EventHandler : virtual public ::IceUtil::Shared
{
public:

    InstancePtr instance() const;
    
    //
    // Return true if the handler is for a datagram transport, false otherwise.
    //
    virtual bool datagram() const = 0;

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
    ICE_API virtual ~EventHandler();

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
