// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public class Server
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("Throughput");
        Ice.Object object = new ThroughputI();
        adapter.add(object, Ice.Util.stringToIdentity("throughput"));
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
            Ice.Properties properties = Ice.Util.createProperties(args);
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(args, properties);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        } finally {
	    if(communicator != null)
	    {
                communicator.destroy();
            }
	}

        System.exit(status);
    }
}
