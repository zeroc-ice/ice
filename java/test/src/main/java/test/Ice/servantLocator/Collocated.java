// Copyright (c) ZeroC, Inc.

package test.Ice.servantLocator;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;

import test.TestHelper;

public class Collocated extends TestHelper {
    @Override
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.servantLocator.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            adapter.addServantLocator(new ServantLocatorI("category"), "category");
            adapter.addServantLocator(new ServantLocatorI(""), "");
            adapter.add(new TestI(), new Identity("asm", ""));
            adapter.add(
                new TestActivationI(), new Identity("activation", "test"));
            AllTests.allTests(this);
        }
    }
}
