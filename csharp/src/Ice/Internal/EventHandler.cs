// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public abstract class EventHandler
{
    //
    // Called to start a new asynchronous read or write operation.
    //
    public abstract bool startAsync(int operation, AsyncCallback completedCallback);

    public abstract bool finishAsync(int operation);

    //
    // Called when there's a message ready to be processed.
    //
    public abstract void message(ThreadPoolCurrent current);

    //
    // Called when the event handler is unregistered.
    //
    public abstract void finished(ThreadPoolCurrent current);

    internal int _ready;
    internal int _pending;
    internal int _started;
    internal bool _finish;

    internal bool _hasMoreData;
    internal int _registered;
}
