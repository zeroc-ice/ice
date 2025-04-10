// Copyright (c) ZeroC, Inc.

package test.Ice.binding;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.Logger;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {
    public void run(String[] args) {
        InitializationData initData = new InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.binding");
        initData.logger =
            new Logger() {
                @Override
                public void print(String message) {}

                @Override
                public void trace(String category, String message) {}

                @Override
                public void warning(String message) {}

                @Override
                public void error(String message) {}

                @Override
                public String getPrefix() {
                    return "NullLogger";
                }

                @Override
                public Logger cloneWithPrefix(String prefix) {
                    return this;
                }
            };

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Identity id = Util.stringToIdentity("communicator");
            adapter.add(new RemoteCommunicatorI(this), id);
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
