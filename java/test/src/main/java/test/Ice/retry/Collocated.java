// Copyright (c) ZeroC, Inc.

package test.Ice.retry;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.Ice.retry.Test.RetryPrx;
import test.TestHelper;

public class Collocated extends TestHelper {
    private final Instrumentation instrumentation = new Instrumentation();

    private void setupObjectAdapter(Communicator communicator) {
        ObjectAdapter adapter = communicator.createObjectAdapter("");
        adapter.add(new RetryI(), Util.stringToIdentity("retry"));
        // adapter.activate(); // Don't activate OA to ensure collocation is used.
    }

    @Override
    public void run(String[] args) {
        InitializationData initData = new InitializationData();
        initData.observer = instrumentation.getObserver();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.retry");
        initData.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");
        //
        // We don't want connection warnings because of the timeout
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        try (Communicator communicator = initialize(initData)) {
            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
            initData.observer = instrumentation.getObserver();
            try (Communicator communicator2 = initialize(initData)) {

                setupObjectAdapter(communicator);
                setupObjectAdapter(communicator2);

                RetryPrx retry =
                        AllTests.allTests(
                                this, communicator, communicator2, instrumentation, "retry");
                retry.shutdown();
            }
        }
    }
}
