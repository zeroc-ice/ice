// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public class Collocated
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

            TestPrx obj = AllTests.allTests(communicator());

            System.out.print("testing whether server is gone... ");
            System.out.flush();
            try
            {
                obj.ice_ping();
                throw new RuntimeException();
            }
            catch(Ice.LocalException ex)
            {
                System.out.println("ok");
            }

            adapter.waitForDeactivate();
            return 0;
        }
    }

    public static void
    main(String[] args)
    {
        TestServer app = new TestServer();
        int result = app.main("Collocated", args);
        System.exit(result);
    }
}
