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

namespace _Ice
{

class EventHandlerI;
void ICE_API _incRef(EventHandlerI*);
void ICE_API _decRef(EventHandlerI*);
typedef _Ice::Handle<EventHandlerI> EventHandler;

}

#endif
