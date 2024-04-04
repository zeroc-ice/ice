// Copyright (c) ZeroC, Inc.

namespace IceInternal;

public abstract class EventHandler
{
    //
    // Called to start a new asynchronous read or write operation.
    //
    abstract public bool startAsync(int op, AsyncCallback cb, ref bool completedSynchronously);

    abstract public bool finishAsync(int op);

    //
    // Called when there's a message ready to be processed.
    //
    abstract public void message(ref ThreadPoolCurrent op);

    //
    // Called when the event handler is unregistered.
    //
    abstract public void finished(ref ThreadPoolCurrent op);

    internal int _ready;
    internal int _pending;
    internal int _started;
    internal bool _finish;

    internal bool _hasMoreData;
    internal int _registered;
}
