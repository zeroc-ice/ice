// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
