// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;

abstract class EventHandler {
    //
    // Called when there's a message ready to be processed.
    //
    public abstract void message(ThreadPoolCurrent current);

    //
    // Called when the event handler is unregistered.
    //
    public abstract void finished(ThreadPoolCurrent current, boolean close);

    //
    // Get a textual representation of the event handler.
    //
    @Override
    public abstract String toString();

    //
    // Get the native information of the handler, this is used by the selector.
    //
    public abstract SelectableChannel fd();

    //
    // Set the ready callback
    //
    public abstract void setReadyCallback(ReadyCallback callback);

    public int _disabled;
    public int _registered;
    public int _ready;
    public SelectionKey _key;
}
