// Copyright (c) ZeroC, Inc.

package test.Ice.binding;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.SocketException;

import test.Ice.binding.Test.RemoteCommunicator;
import test.Ice.binding.Test.RemoteObjectAdapterPrx;
import test.TestHelper;

public class RemoteCommunicatorI implements RemoteCommunicator {
    public RemoteCommunicatorI(TestHelper helper) {
        _helper = helper;
    }

    @Override
    public RemoteObjectAdapterPrx createObjectAdapter(
            String name, String endpts, Current current) {
        int retry = 5;
        while (true) {
            try {
                String endpoints = endpts;
                if (endpoints.indexOf("-p") < 0) {
                    endpoints = _helper.getTestEndpoint(_nextPort++, endpoints);
                }
                Communicator com = current.adapter.getCommunicator();
                com.getProperties().setProperty(name + ".ThreadPool.Size", "1");
                ObjectAdapter adapter =
                        com.createObjectAdapterWithEndpoints(name, endpoints);
                return RemoteObjectAdapterPrx.uncheckedCast(
                        current.adapter.addWithUUID(new RemoteObjectAdapterI(adapter)));
            } catch (SocketException ex) {
                if (--retry == 0) {
                    throw ex;
                }
            }
        }
    }

    @Override
    public void deactivateObjectAdapter(
            RemoteObjectAdapterPrx adapter, Current current) {
        adapter.deactivate(); // Collocated call.
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    private final TestHelper _helper;
    private int _nextPort = 10;
}
