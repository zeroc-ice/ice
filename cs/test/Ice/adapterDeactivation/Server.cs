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

public class Server
{
    internal class TestServer : Ice.Application
    {
        public override int run(string[] args)
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
    
    public static void Main(string[] args)
    {
        TestServer app = new TestServer();
        int result = app.main(args);
        System.Environment.Exit(result);
    }
}
