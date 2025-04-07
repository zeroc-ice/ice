// Copyright (c) ZeroC, Inc.

package test.Ice.adapterDeactivation;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;

import test.Ice.adapterDeactivation.Test.*;

public final class TestI implements TestIntf {
    public void transientOp(Current current) {
        Communicator communicator = current.adapter.getCommunicator();

        ObjectAdapter adapter =
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
        adapter.activate();
        adapter.destroy();
    }

    public void deactivate(Current current) {
        current.adapter.deactivate();
        try {
            Thread.sleep(100);
        } catch (InterruptedException ex) {
        }
    }
}
