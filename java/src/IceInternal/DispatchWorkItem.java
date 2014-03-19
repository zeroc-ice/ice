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
    public DispatchWorkItem(Instance instance)
    {
        _instance = instance;
    }
    
    final public void execute(ThreadPoolCurrent current)
    {
        Ice.Dispatcher dispatcher = _instance.initializationData().dispatcher;
        if(dispatcher != null)
        {
            try
            {
                dispatcher.dispatch(this, null);
            }
            catch(java.lang.Exception ex)
            {
                if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    ex.printStackTrace(pw);
                    pw.flush();
                    _instance.initializationData().logger.warning("dispatch exception:\n" + sw.toString());
                }
            }
        }
        else
        {
            current.ioCompleted(); // Promote a follower.
            this.run();
        }
    }

    private Instance _instance;
}
