// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package IceInternal;

abstract class EventHandler
{
    //
    // Return true if read() must be called before calling message().
    //
    abstract public boolean readable();

    //
    // Read data via the event handler. May only be called if
    // readable() returns true.
    //
    abstract public void read(BasicStream is);

    //
    // A complete message has been received.
    //
    abstract public void message(BasicStream stream, ThreadPool threadPool);

    //
    // Will be called if the event handler is finally
    // unregistered. (Calling unregister() does not unregister
    // immediately.)
    //
    abstract public void finished(ThreadPool threadPool);

    //
    // Propagate an exception to the event handler.
    //
    abstract public void exception(Ice.LocalException ex);

    //
    // Get a textual representation of the event handler.
    //
    abstract public String toString();

    protected
    EventHandler(Instance instance)
    {
        _instance = instance;
        _stream = new BasicStream(instance);
    }

    protected void
    finalize()
        throws Throwable
    {
	_stream.destroy();
    }

    protected Instance _instance;

    //
    // The _stream data member is for use by ThreadPool only.
    //
    BasicStream _stream;
}
