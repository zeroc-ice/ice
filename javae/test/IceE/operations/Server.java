// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    public static int
    run(String[] args, Ice.Communicator communicator)
    {
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object object = new MyDerivedClassI(adapter, Ice.Util.stringToIdentity("test"));
        adapter.add(object, Ice.Util.stringToIdentity("test"));
        adapter.activate();

	//
	// Make a separate adapter. We use this to test that Ice::Context is correctly passed
	// to checkedCast() operation.
	//
	communicator.getProperties().setProperty("CheckedCastAdapter.Endpoints", "default -p 12346 -t 10000");
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
            status = run(args, communicator);
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
