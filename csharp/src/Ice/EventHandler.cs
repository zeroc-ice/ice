// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

using System;

public abstract class EventHandler
{
    //
    // Called to start a new asynchronous read or write operation.
    //
    abstract public bool startAsync(int op, AsyncCallback cb, ref bool completedSynchronously);

    abstract public bool finishAsync(int op);

    //
    // Called when there's a message ready to be processed.
    //
    abstract public void message(ref ThreadPoolCurrent op);

    //
    // Called when the event handler is unregistered.
    //
    abstract public void finished(ref ThreadPoolCurrent op);

    internal int _ready = 0;
    internal int _pending = 0;
    internal int _started = 0;
    internal bool _finish = false;
    
    internal bool _hasMoreData = false;
    internal int _registered = 0;
}

}
