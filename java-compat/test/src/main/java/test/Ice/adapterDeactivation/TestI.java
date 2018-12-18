// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.adapterDeactivation;

import test.Ice.adapterDeactivation.Test.*;

public final class TestI extends _TestIntfDisp
{
    public void
    _transient(Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();

        Ice.ObjectAdapter adapter =
            communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
        adapter.activate();
        adapter.destroy();
    }

    public void
    deactivate(Ice.Current current)
    {
        current.adapter.deactivate();
        try
        {
            Thread.sleep(100);
        }
        catch(InterruptedException ex)
        {
        }
    }
}
