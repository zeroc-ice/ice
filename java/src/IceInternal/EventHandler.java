// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

abstract class EventHandler
{
    //
    // Return true if read() must be called before calling message().
    //
    abstract boolean readable();

    //
    // Read data via the event handler. May only be called if
    // readable() returns true.
    //
    abstract void read(BasicStream is);

    //
    // A complete message has been received.
    //
    abstract void message(BasicStream stream, ThreadPool threadPool);

    //
    // Will be called if the event handler is finally
    // unregistered. (Calling unregister() does not unregister
    // immediately.)
    //
    abstract void finished(ThreadPool threadPool);

    //
    // Propagate an exception to the event handler.
    //
    abstract void exception(Ice.LocalException ex);

    protected
    EventHandler(Instance instance)
    {
        _instance = instance;
        _stream = new BasicStream(instance);
    }

    protected Instance _instance;

    //
    // The _stream data member is for use by ThreadPool only
    //
    BasicStream _stream;
}
