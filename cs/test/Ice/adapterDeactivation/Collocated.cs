// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
