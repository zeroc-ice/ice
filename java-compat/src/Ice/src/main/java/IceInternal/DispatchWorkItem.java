// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

//
// A helper class for thread pool work items that only need to call user
// callbacks. If a dispatcher is installed with the communicator, the
// thread pool work item is executed with the dispatcher, otherwise it's
// executed by a thread pool thread (after promoting a follower thread).
//
abstract public class DispatchWorkItem implements ThreadPoolWorkItem, Runnable
{
    public DispatchWorkItem()
    {
    }

    public DispatchWorkItem(Ice.Connection connection)
    {
        _connection = connection;
    }

    @Override
    final public void
    execute(ThreadPoolCurrent current)
    {
        current.ioCompleted(); // Promote a follower
        current.dispatchFromThisThread(this);
    }

    public Ice.Connection
    getConnection()
    {
        return _connection;
    }

    private Ice.Connection _connection;
}
