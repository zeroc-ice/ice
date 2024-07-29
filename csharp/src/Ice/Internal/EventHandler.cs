// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public abstract class EventHandler
{
    //
    // Called to start a new asynchronous read or write operation.
    //
    abstract public bool startAsync(int op, AsyncCallback cb);

    abstract public bool finishAsync(int op);

    //
    // Called when there's a message ready to be processed.
    //
    abstract public void message(ThreadPoolCurrent op);

    //
    // Called when the event handler is unregistered.
    //
    abstract public void finished(ThreadPoolCurrent op);

    internal int _ready;
    internal int _pending;
    internal int _started;
    internal bool _finish;

    internal bool _hasMoreData;
    internal int _registered;
}
