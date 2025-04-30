// Copyright (c) ZeroC, Inc.

package test.Ice.slicing.objects;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ClassSliceLoader;
import com.zeroc.Ice.CompositeSliceLoader;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.Ice.slicing.objects.client.Test.CompactPCDerived;
import test.Ice.slicing.objects.client.Test.CompactPDerived;
import test.Ice.slicing.objects.client.Test.TestIntfPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.SliceLoader.NotFoundCacheSize", "5");
        initData.properties.setProperty("Ice.Warn.SliceLoader", "0"); // comment out to see the warning
        initData.sliceLoader = new CompositeSliceLoader(
            new AllTests.CustomSliceLoader(),
            new ClassSliceLoader(CompactPDerived.class, CompactPCDerived.class),
            new ModuleToPackageSliceLoader("::Test", "test.Ice.slicing.objects.client.Test"));

        try (Communicator communicator = initialize(initData)) {
            TestIntfPrx test = AllTests.allTests(this, false);
            test.shutdown();
        }
    }
}
