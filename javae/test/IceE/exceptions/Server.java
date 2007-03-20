// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    public static int
    run(String[] args, Ice.Communicator communicator, Ice.InitializationData data, java.io.PrintStream out)
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
        Ice.Object object = new ThrowerI(adapter);
        adapter.add(object, communicator.stringToIdentity("thrower"));
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
	    //
	    // For this test, we need a dummy logger, otherwise the
	    // assertion test will print an error message.
    	    //
	    Ice.InitializationData initData = new Ice.InitializationData();
	    initData.logger = new DummyLogger();
            communicator = Ice.Util.initialize(args, initData);
            status = run(args, communicator, initData, System.out);
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
