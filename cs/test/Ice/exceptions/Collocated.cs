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
    private static int
    run(string[] args, Ice.Communicator communicator)
    {
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 2000");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object obj = new ThrowerI(adapter);
        adapter.add(obj, Ice.Util.stringToIdentity("thrower"));
        AllTests.allTests(communicator, true);
	return 0;
    }

    public static void
    Main(string[] args)
    {
	int status = 0;
	Ice.Communicator communicator = null;
	
	try
	{
	    communicator = Ice.Util.initialize(ref args);
	    status = run(args, communicator);
	}
	catch(System.Exception ex)
	{
	    System.Console.WriteLine(ex);
	    status = 1;
	}
	
	if(communicator != null)
	{
	    try
	    {
		communicator.destroy();
	    }
	    catch(Ice.LocalException ex)
	    {
		System.Console.WriteLine(ex);
		status = 1;
	    }
	}
	
	System.Environment.Exit(status);
    }
}
