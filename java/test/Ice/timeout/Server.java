// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    public static int
    run(String[] args, Ice.Communicator communicator, java.io.PrintStream out)
    {
	//
	// When running as a MIDlet the properties for the server may be
	// overridden by configuration. If it isn't then we assume
	// defaults.
	//
	if(communicator.getProperties().getProperty("TestAdapter.Endpoints").length() == 0)
	{
	    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");
	}

	Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
	adapter.add(new TimeoutI(), Ice.Util.stringToIdentity("timeout"));
	adapter.activate();

	communicator.waitForShutdown();
	return 0;
    }

    public static void
    main(String[] args)
    {
	int status = 0;
	Ice.Communicator communicator = null;

	try
	{
	    Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
	    Ice.Properties properties = Ice.Util.getDefaultProperties(argsH);

	    //
	    // This test kills connections, so we don't want warnings.
	    //
	    properties.setProperty("Ice.Warn.Connections", "0");

	    communicator = Ice.Util.initialize(argsH);
	    status = run(argsH.value, communicator, System.out);
	}
	catch(Ice.LocalException ex)
	{
	    ex.printStackTrace();
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
		ex.printStackTrace();
		status = 1;
	    }
	}

	System.gc();
	System.exit(status);
    }
}
