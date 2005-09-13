// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class Client extends Ice.Application
{
    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        final String refProperty = "Latency.Ping";
        String ref = properties.getProperty(refProperty);
        if(ref.length() == 0)
        {
            System.err.println("property `" + refProperty + "' not set");
            return 1;
        }

        PingPrx ping = PingPrxHelper.checkedCast(communicator().stringToProxy(ref));
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
        Client app = new Client();
        int status = app.main("Client", args, "config");
        System.exit(status);
    }
}
