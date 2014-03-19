// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    abstract public void finished(ThreadPoolCurrent current);

    //
    // Get a textual representation of the event handler.
    //
    abstract public String toString();

    //
    // Get the native information of the handler, this is used by the selector.
    //
    abstract public java.nio.channels.SelectableChannel fd();

    //
    // In Java, it's possible that the transceiver reads more data than what was 
    // really asked. If this is the case, hasMoreData() returns true and the handler
    // read() method should be called again (without doing a select()). This is 
    // handled by the Selector class (it adds the handler to a separate list of 
    // handlers if this method returns true.)
    //
    abstract public boolean hasMoreData();

    int _disabled = 0;
    int _registered = 0;
    int _ready = 0;
    java.nio.channels.SelectionKey _key = null;
}
