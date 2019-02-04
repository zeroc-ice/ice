//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.servantLocator;

public class Collocated extends test.TestHelper
{
    @Override
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.servantLocator");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            adapter.addServantLocator(new ServantLocatorI("category"), "category");
            adapter.addServantLocator(new ServantLocatorI(""), "");
            adapter.add(new TestI(), com.zeroc.Ice.Util.stringToIdentity("asm"));
            adapter.add(new TestActivationI(), com.zeroc.Ice.Util.stringToIdentity("test/activation"));
            AllTests.allTests(this);
        }
    }
}
