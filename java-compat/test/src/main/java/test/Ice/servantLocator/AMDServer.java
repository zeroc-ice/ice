// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.servantLocator;

public class AMDServer extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.servantLocator.AMD");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            adapter.addServantLocator(new AMDServantLocatorI("category"), "category");
            adapter.addServantLocator(new AMDServantLocatorI(""), "");
            adapter.add(new AMDTestI(), Ice.Util.stringToIdentity("asm"));
            adapter.add(new AMDTestActivationI(), Ice.Util.stringToIdentity("test/activation"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
