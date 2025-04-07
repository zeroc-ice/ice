// Copyright (c) ZeroC, Inc.

package test.IceDiscovery.simple;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class Server extends TestHelper {
    public void run(String[] args) {
        List<String> rargs = new ArrayList<String>();
        Properties properties = createTestProperties(args, rargs);
        try (Communicator communicator = initialize(properties)) {
            int num = Integer.parseInt(rargs.get(0));
            communicator
                    .getProperties()
                    .setProperty("ControlAdapter.Endpoints", getTestEndpoint(num));
            communicator.getProperties().setProperty("ControlAdapter.AdapterId", "control" + num);
            communicator.getProperties().setProperty("ControlAdapter.ThreadPool.Size", "1");

            ObjectAdapter adapter =
                    communicator().createObjectAdapter("ControlAdapter");
            adapter.add(new ControllerI(), Util.stringToIdentity("controller" + num));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
