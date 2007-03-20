// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Collocated
{
    public static int
    run(String[] args, Ice.Communicator communicator, Ice.InitializationData data, java.io.PrintStream out)
    {
	communicator.getProperties().setProperty("Test.Proxy", "thrower:default -p 12010 -t 10000"); 
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object object = new ThrowerI(adapter);
        adapter.add(object, communicator.stringToIdentity("thrower"));
	adapter.activate();

        AllTests.allTests(communicator, out);

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

        System.exit(status);
    }
}
