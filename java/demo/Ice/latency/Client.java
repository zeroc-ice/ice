// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
        if (ref == null)
        {
            System.err.println("property `" + refProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        PingPrx ping = PingPrxHelper.checkedCast(base);
        if (ping == null)
        {
            System.err.println("invalid object reference");
            return 1;
        }

        // Initial ping to setup the connection.
        ping.ice_ping();

        long tv1 = System.currentTimeMillis();
        final int repetitions = 100000;
        System.out.println("pinging server " + repetitions + " times (this may take a while)");
        for (int i = 0; i < repetitions; i++)
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
            Ice.Properties properties = Ice.Util.createPropertiesFromFile(args, "config");
            communicator = Ice.Util.initializeWithProperties(properties);
            status = run(args, communicator);
        }
        catch (Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if (communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch (Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.exit(status);
    }
}
