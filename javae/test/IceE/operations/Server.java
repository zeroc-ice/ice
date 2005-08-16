// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
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
	    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345");
	}

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object object = new MyDerivedClassI(adapter, Ice.Util.stringToIdentity("test"));
        adapter.add(object, Ice.Util.stringToIdentity("test"));
        adapter.activate();

	//
	// Make a separate adapter. We use this to test that
	// Ice::Context is correctly passed to checkedCast() operation.
	//
	if(communicator.getProperties().getProperty("CheckedCastAdapter.Endpoints").length() == 0)
	{
	    communicator.getProperties().setProperty("CheckedCastAdapter.Endpoints", "default -p 12346 -t 10000");
	}

	adapter = communicator.createObjectAdapter("CheckedCastAdapter");
        object = new TestCheckedCastI();
        adapter.add(object, Ice.Util.stringToIdentity("test"));
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
            communicator = Ice.Util.initialize(args);
            status = run(args, communicator, System.out);
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
