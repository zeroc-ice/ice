// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class AMDServer extends TestHelper {
    @Override
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.seqMapping.Test");
        initData.properties = createTestProperties(args);

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new AMDMyClassI(), new Identity("test", ""));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
