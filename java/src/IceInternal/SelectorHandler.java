// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

abstract class SelectorHandler
{
    abstract public java.nio.channels.SelectableChannel fd();

    //
    // In Java, it's possible that the transceiver reads more data
    // than what was really asked. If this is the case, hasMoreData()
    // returns true and the handler read() method should be called 
    // again (without doing a select()). This is handled by the 
    // Selector class (it adds the handler to a separate list of handlers
    // if this method returns true.)
    //
    abstract public boolean hasMoreData();

    //
    // The _key data member are only for use by the Selector.
    //
    protected java.nio.channels.SelectionKey _key;
    protected SocketStatus _pendingStatus;
};
