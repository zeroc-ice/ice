// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

class EventHandlerOpPair
{
    EventHandlerOpPair(EventHandler handler, int op)
    {
        this.handler = handler;
        this.op = op;
    }

    EventHandler handler;
    int op;
}
