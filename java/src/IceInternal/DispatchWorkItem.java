// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    final public void execute(ThreadPoolCurrent current)
    {
        Instance instance = current.stream.instance();
        Ice.Dispatcher dispatcher = instance.initializationData().dispatcher;
        if(dispatcher != null)
        {
            try
            {
                dispatcher.dispatch(this, _connection);
            }
            catch(java.lang.Exception ex)
            {
                if(instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    ex.printStackTrace(pw);
                    pw.flush();
                    instance.initializationData().logger.warning("dispatch exception:\n" + sw.toString());
                }
            }
        }
        else
        {
            current.ioCompleted(); // Promote a follower.
            this.run();
        }
    }

    private Ice.Connection _connection;
}
