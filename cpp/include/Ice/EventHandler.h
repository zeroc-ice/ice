// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_EVENT_HANDLER_H
#define ICE_EVENT_HANDLER_H

#include <Ice/EventHandlerF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class ICE_API EventHandlerI : public Shared
{
public:
    
    virtual int fd() = 0; // Filedescriptor
    virtual bool server() = 0; // True if server-side event handler
    virtual void receive() = 0; // Read to receive data
    virtual void finished() = 0; // Event handler now unregistered

protected:
    
    EventHandlerI() { }
    virtual ~EventHandlerI() { }

private:

    EventHandlerI(const EventHandlerI&);
    void operator=(const EventHandlerI&);
};

}

#endif
