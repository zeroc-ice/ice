// Copyright (c) ZeroC, Inc.

package test.Ice.retry;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;

import test.Ice.retry.Test.RetryPrx;
import test.TestHelper;

public class Client extends TestHelper {
    private final Instrumentation instrumentation = new Instrumentation();

    public void run(String[] args) {
        InitializationData initData = new InitializationData();
        initData.observer = instrumentation.getObserver();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.RetryIntervals", "0 1 400 1");
        // We don't want connection warnings because of the timeout
        initData.properties.setProperty("Ice.Warn.Connections", "0");

        try (Communicator communicator = initialize(initData)) {
            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.RetryIntervals", "0 1 10000");

            try (Communicator communicator2 = initialize(initData)) {
                RetryPrx retry =
                    AllTests.allTests(
                        this,
                        communicator,
                        communicator2,
                        instrumentation,
                        "retry:" + getTestEndpoint(0));
                retry.shutdown();
            }
        }
    }
}
