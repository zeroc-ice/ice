// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_EVENT_HANDLER_F_H
#define ICE_EVENT_HANDLER_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class EventHandlerI;
void ICE_API incRef(EventHandlerI*);
void ICE_API decRef(EventHandlerI*);
typedef __Ice::Handle<EventHandlerI> EventHandler;

}

#endif
