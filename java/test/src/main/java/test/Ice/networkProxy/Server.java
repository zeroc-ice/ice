// Copyright (c) ZeroC, Inc.

package test.Ice.networkProxy;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.Ice.networkProxy.Test.TestIntf;
import test.TestHelper;

public class Server extends TestHelper {
    public static class TestI implements TestIntf {
        public void shutdown(Current current) {
            current.adapter.getCommunicator().shutdown();
        }
    }

    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestI(), Util.stringToIdentity("test"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
