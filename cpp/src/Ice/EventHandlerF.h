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

namespace IceInternal
{

class EventHandler;
void incRef(EventHandler*);
void decRef(EventHandler*);
typedef Handle<EventHandler> EventHandlerPtr;

}

#endif
