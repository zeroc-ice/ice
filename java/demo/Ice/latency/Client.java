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


public class Client
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();
        final String refProperty = "Latency.Ping";
        String ref = properties.getProperty(refProperty);
        if(ref.length() == 0)
        {
            System.err.println("property `" + refProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        PingPrx ping = PingPrxHelper.checkedCast(base);
        if(ping == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        // Initial ping to setup the connection.
        ping.ice_ping();

        long tv1 = System.currentTimeMillis();
        final int repetitions = 100000;
        System.out.println("pinging server " + repetitions + " times (this may take a while)");
        for(int i = 0; i < repetitions; i++)
        {
            ping.ice_ping();
        }

        long tv2 = System.currentTimeMillis();
        double total = (double)(tv2 - tv1);
        double perPing = total / repetitions;

        System.out.println("time for " + repetitions + " pings: " + total + "ms");
        System.out.println("time per ping: " + perPing + "ms");

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
