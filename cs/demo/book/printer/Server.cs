// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class PrinterI : Demo._PrinterDisp
{
    public override void printString(string s, Ice.Current current)
    {
        Console.WriteLine(s);
    }
}

public class Server
{
    public static void Main(string[] args)
    {
	int status = 0;
	Ice.Communicator ic = null;
        try {
	    ic = Ice.Util.initialize(ref args);
	    Ice.ObjectAdapter adapter
		= ic.createObjectAdapterWithEndpoints(
		    "SimplePrinterAdapter", "default -p 10000");
	    Ice.Object obj = new PrinterI();
	    adapter.add(
		    obj,
		    Ice.Util.stringToIdentity("SimplePrinter"));
	    adapter.activate();
	    ic.waitForShutdown();
        } catch (Exception e) {
	    Console.Error.WriteLine(e);
	    status = 1;
	}
	if (ic != null)
	{
	    // Clean up
	    //
	    try {
		ic.destroy();
	    } catch (Exception e) {
		Console.Error.WriteLine(e);
		status = 1;
	    }
	}
        Environment.Exit(status);
    }
}
