// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.admin;

import test.Ice.admin.Test.*;

public class RemoteCommunicatorI extends _RemoteCommunicatorDisp implements Ice.PropertiesAdminUpdateCallback
{
    RemoteCommunicatorI(Ice.Communicator communicator)
    {
        _communicator = communicator;
        _called = false;
    }

    public Ice.ObjectPrx getAdmin(Ice.Current current)
    {
        return _communicator.getAdmin();
    }

    public synchronized java.util.Map<String, String> getChanges(Ice.Current current)
    {
        //
        // The client calls PropertiesAdmin::setProperties() and then invokes
        // this operation. Since setProperties() is implemented using AMD, the
        // client might receive its reply and then call getChanges() before our
        // updated() method is called. We block here to ensure that updated()
        // gets called before we return the most recent set of changes.
        //
        while(!_called)
        {
            try
            {
                wait();
            }
            catch(InterruptedException ex)
            {
            }
        }

        _called = false;

        return _changes;
    }

    public void shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    public void waitForShutdown(Ice.Current current)
    {
        //
        // Note that we are executing in a thread of the *main* communicator,
        // not the one that is being shut down.
        //
        _communicator.waitForShutdown();
    }

    public void destroy(Ice.Current current)
    {
        _communicator.destroy();
    }

    public synchronized void updated(java.util.Map<String, String> changes)
    {
        _changes = changes;
        _called = true;
        notify();
    }

    private Ice.Communicator _communicator;
    private java.util.Map<String, String> _changes;
    private boolean _called;
}
