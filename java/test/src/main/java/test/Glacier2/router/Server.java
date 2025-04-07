// Copyright (c) ZeroC, Inc.

package test.Glacier2.router;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Server extends TestHelper {

    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Glacier2.router");
        try (Communicator communicator = initialize(properties)) {
            communicator
                    .getProperties()
                    .setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter =
                    communicator.createObjectAdapter("CallbackAdapter");

            // The test allows "c1" as category.
            adapter.add(new CallbackI(), Util.stringToIdentity("c1/callback"));

            // The test allows "c2" as category.
            adapter.add(new CallbackI(), Util.stringToIdentity("c2/callback"));

            // The test rejects "c3" as category.
            adapter.add(new CallbackI(), Util.stringToIdentity("c3/callback"));

            // The test allows the prefixed userid.
            adapter.add(new CallbackI(), Util.stringToIdentity("_userid/callback"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }
}
