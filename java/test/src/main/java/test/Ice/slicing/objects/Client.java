// Copyright (c) ZeroC, Inc.

package test.Ice.slicing.objects;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ClassSliceLoader;
import com.zeroc.Ice.Properties;

import test.Ice.slicing.objects.client.Test.CompactPCDerived;
import test.Ice.slicing.objects.client.Test.CompactPDerived;
import test.Ice.slicing.objects.client.Test.TestIntfPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.slicing.objects.client");
        initData.sliceLoader = new ClassSliceLoader(CompactPDerived.class, CompactPCDerived.class);

        try (Communicator communicator = initialize(initData)) {
            TestIntfPrx test = AllTests.allTests(this, false);
            test.shutdown();
        }
    }
}
