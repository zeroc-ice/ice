// Copyright (c) ZeroC, Inc.

package test.Ice.servantLocator;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class AMDServer extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.servantLocator.AMD.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.addServantLocator(new AMDServantLocatorI("category"), "category");
            adapter.addServantLocator(new AMDServantLocatorI(""), "");
            adapter.add(new AMDTestI(), Util.stringToIdentity("asm"));
            adapter.add(
                new AMDTestActivationI(),
                Util.stringToIdentity("test/activation"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
