// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public class Collocated
{
    internal class TestClient : Ice.Application
    {
        public override int run(string[] args)
        {
	    communicator().getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");
	    Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
	    Ice.ServantLocator locator = new ServantLocatorI();
	    adapter.addServantLocator(locator, "");

            AllTests.allTests(communicator());
            
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
