// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public class Server
{
    static class TestServer extends Ice.Application
    {
        public int
        run(String[] args)
        {
            communicator().getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
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
