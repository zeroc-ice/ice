// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class Server
{
    static class TestServer extends Ice.Application
    {
        public int
        run(String[] args)
        {
            communicator().getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 2000");
            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            Ice.ServantLocator locator = new ServantLocatorI();
            adapter.addServantLocator(locator, "");
            adapter.activate();
            adapter.waitForDeactivate();
            return 0;
        }
    }

    public static void
    main(String[] args)
    {
        TestServer app = new TestServer();
        int result = app.main("Server", args);
        System.exit(result);
    }
}
