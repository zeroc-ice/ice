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
    // Returns true if the event handler belongs to the server-side of
    // an application. Client-side otherwise.
    //
    abstract boolean server();

    //
    // Return true if read() must be called before calling message().
    //
    abstract boolean readable();

    //
    // Read data via the event handler. May only be called if
    // readable() returns true.
    //
    abstract void read(Ice.Stream is);

    //
    // A complete message has been received.
    //
    abstract void message(Ice.Stream stream);

    //
    // Signal exception during reading or unmarshaling.
    //
    abstract void exception(Ice.LocalException ex);

    //
    // Will be called if the event handler is finally
    // unregistered. (Calling unregister() does not unregister
    // immediately.)
    //
    abstract void finished();
    
    //
    // Try to destroy the event handler. Returns false if the event
    // handler cannot be destroyed because it is in use, or true
    // otherwise.
    //
    abstract boolean tryDestroy();

    protected
    EventHandler(Instance instance)
    {
        _instance = instance;
        _stream = new StreamI(instance);
    }

    protected Instance _instance;

    //
    // The _stream data member is for use by ThreadPool only
    //
    Ice.Stream _stream;
}
