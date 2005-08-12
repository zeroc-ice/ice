// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Collocated
{
    public static int
    run(String[] args, Ice.Communicator communicator, java.io.PrintStream out)
    {
	communicator.getProperties().setProperty("Test.Proxy", "test:default -p 12345 -t 10000");
	communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
	Ice.Object object = new TestI();
	adapter.add(object, Ice.Util.stringToIdentity("test"));
	adapter.activate();

        AllTests.allTests(communicator, out);

        adapter.waitForDeactivate();
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

        System.exit(status);
    }
}
