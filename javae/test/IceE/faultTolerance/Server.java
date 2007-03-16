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
    private static void
    usage()
    {
        System.err.println("Usage: Server port");
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        int port = 0;
        for(int i = 0; i < args.length; i++)
        {
            if(args[i].charAt(0) == '-')
            {
                System.err.println("Server: unknown option `" + args[i] + "'");
                usage();
                return 1;
            }

            if(port > 0)
            {
                System.err.println("Server: only one port can be specified");
                usage();
                return 1;
            }

            try
            {
                port = Integer.parseInt(args[i]);
            }
            catch(NumberFormatException ex)
            {
                System.err.println("Server: invalid port");
                usage();
                return 1;
            }
        }

        if(port <= 0)
        {
            System.err.println("Server: no port specified");
            usage();
            return 1;
        }

        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p " + port);
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object object = new TestI(adapter, port);
        adapter.add(object, communicator.stringToIdentity("test"));
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
	    // In this test, we need a longer server idle time,
	    // otherwise our test servers may time out before they are
	    // used in the test.
	    //
	    Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
	    Ice.InitializationData initData = new Ice.InitializationData();
	    initData.properties = Ice.Util.createProperties(argsH);
	    initData.properties.setProperty("Ice.ServerIdleTime", "120"); // Two minutes.

            communicator = Ice.Util.initialize(argsH, initData);
            status = run(argsH.value, communicator);
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
