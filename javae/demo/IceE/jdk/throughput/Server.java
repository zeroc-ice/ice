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
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        //
        // Check if we need to run with small sequences.
        //
        int reduce = 1;
        for(int i = 0; i < args.length; ++i)
        {
            if(args[i].equals("--small"))
            {
                reduce = 100;
            }
        }

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("Throughput");
        Ice.Object object = new ThroughputI(reduce);
        adapter.add(object, communicator.stringToIdentity("throughput"));
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
	    Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.load("config");
            communicator = Ice.Util.initialize(args, initData);
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

        System.exit(status);
    }
}
