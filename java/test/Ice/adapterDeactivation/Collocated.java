// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class Collocated
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
