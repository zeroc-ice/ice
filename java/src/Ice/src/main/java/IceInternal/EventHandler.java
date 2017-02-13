// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class EventHandler
{
    //
    // Called when there's a message ready to be processed.
    //
    abstract public void message(ThreadPoolCurrent current);

    //
    // Called when the event handler is unregistered.
    //
    abstract public void finished(ThreadPoolCurrent current, boolean close);

    //
    // Get a textual representation of the event handler.
    //
    @Override
    abstract public String toString();

    //
    // Get the native information of the handler, this is used by the selector.
    //
    abstract public java.nio.channels.SelectableChannel fd();

    public int _disabled = 0;
    public Ice.Holder<Boolean> _hasMoreData = new Ice.Holder<Boolean>(false);
    public int _registered = 0;
    public java.nio.channels.SelectionKey _key = null;
}
