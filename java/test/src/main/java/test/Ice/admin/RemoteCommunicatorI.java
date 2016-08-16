// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
    }

    @Override
    public Ice.ObjectPrx getAdmin(Ice.Current current)
    {
        return _communicator.getAdmin();
    }

    @Override
    public synchronized java.util.Map<String, String> getChanges(Ice.Current current)
    {
        return _changes;
    }

    @Override
    public void print(String message, Ice.Current current)
    {
        _communicator.getLogger().print(message);
    }

    @Override
    public void trace(String category, String message, Ice.Current current)
    {
        _communicator.getLogger().trace(category, message);
    }

    @Override
    public void warning(String message, Ice.Current current)
    {
        _communicator.getLogger().warning(message);
    }

    @Override
    public void error(String message, Ice.Current current)
    {
        _communicator.getLogger().error(message);
    }

    @Override
    public void shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    @Override
    public void waitForShutdown(Ice.Current current)
    {
        //
        // Note that we are executing in a thread of the *main* communicator,
        // not the one that is being shut down.
        //
        _communicator.waitForShutdown();
    }

    @Override
    public void destroy(Ice.Current current)
    {
        _communicator.destroy();
    }

    @Override
    public synchronized void updated(java.util.Map<String, String> changes)
    {
        _changes = changes;
    }

    private Ice.Communicator _communicator;
    private java.util.Map<String, String> _changes;
}
