// Copyright (c) ZeroC, Inc.

package test.Ice.hold;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

import java.util.Timer;

public class Server extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.hold");
        try (Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter1.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Size", "5");
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Serialize", "0");

            communicator.getProperties().setProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Size", "5");
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Serialize", "1");

            Timer timer = new Timer();

            ObjectAdapter adapter1 = communicator.createObjectAdapter("TestAdapter1");
            adapter1.add(new HoldI(timer, adapter1), Util.stringToIdentity("hold"));

            ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
            adapter2.add(new HoldI(timer, adapter2), Util.stringToIdentity("hold"));

            adapter1.activate();
            adapter2.activate();

            serverReady();
            communicator.waitForShutdown();

            timer.cancel();
        }
    }
}
