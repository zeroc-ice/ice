// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        PingPrx ping = PingPrxHelper.checkedCast(communicator().propertyToProxy("Ping.Proxy"));
        if(ping == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        //
        // A method needs to be invoked thousands of times before the JIT compiler
        // will convert it to native code. To ensure an accurate latency measurement,
        // we need to "warm up" the JIT compiler.
        //
        {
            final int repetitions = 20000;
            System.out.print("warming up the JIT compiler...");
            System.out.flush();
            for(int i = 0; i < repetitions; i++)
            {
                ping.ice_ping();
            }
            System.out.println(" ok");
        }

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
        int status = app.main("Client", args, "config.client");
        System.exit(status);
    }
}
