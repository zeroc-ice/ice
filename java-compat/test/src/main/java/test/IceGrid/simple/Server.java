// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceGrid.simple;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
        Ice.Properties properties = createTestProperties(argsH);
        //
        // Its possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we supress this warning.
        //
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Package.Test", "test.IceGrid.simple");
        properties.parseCommandLineOptions("TestAdapter", argsH.value);
        try(Ice.Communicator communicator = initialize(properties))
        {
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.Object object = new TestI();
            String id = communicator.getProperties().getPropertyWithDefault("Identity", "test");
            adapter.add(object, Ice.Util.stringToIdentity(id));
            //shutdownOnInterrupt();
            try
            {
                adapter.activate();
            }
            catch(Ice.ObjectAdapterDeactivatedException ex)
            {
            }
            communicator.waitForShutdown();
            //defaultInterrupt();
        }
    }
}
