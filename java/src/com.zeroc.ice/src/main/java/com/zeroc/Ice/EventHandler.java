// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

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
    public abstract java.nio.channels.SelectableChannel fd();

    //
    // Set the ready callback
    //
    public abstract void setReadyCallback(ReadyCallback callback);

    public int _disabled = 0;
    public int _registered = 0;
    public int _ready = 0;
    public java.nio.channels.SelectionKey _key = null;
}
