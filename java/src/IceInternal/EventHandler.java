// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class EventHandler
{
    //
    // Return true if the handler is for a datagram transport, false otherwise.
    //
    abstract public boolean datagram();

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

    public IceInternal.Instance
    instance()
    {
        return _instance;
    }

    public void
    destroy()
    {
	_stream.destroy();
	_stream = null;
    }

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
	assert(_stream == null);
    }

    protected Instance _instance;

    //
    // The _stream data member is for use by ThreadPool only.
    //
    BasicStream _stream;
}
