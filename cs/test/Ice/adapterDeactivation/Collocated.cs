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
    internal class TestClient : Ice.Application
    {
        public override int run(string[] args)
        {
	    communicator().getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
	    Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
	    Ice.ServantLocator locator = new ServantLocatorI();
	    adapter.addServantLocator(locator, "");

            TestPrx obj = AllTests.allTests(communicator());
            
            System.Console.Out.Write("testing whether server is gone... ");
            System.Console.Out.Flush();
            try
            {
                obj.ice_ping();
                throw new System.Exception();
            }
            catch(Ice.LocalException)
            {
                System.Console.Out.WriteLine("ok");
            }
	    catch(System.Exception ex)
	    {
	        System.Console.Error.WriteLine(ex);
		return 1;
	    }
            
	    adapter.waitForDeactivate();
            return 0;
        }
    }
    
    public static void Main(string[] args)
    {
        TestClient app = new TestClient();
        int result = app.main(args);
        System.Environment.Exit(result);
    }
}
