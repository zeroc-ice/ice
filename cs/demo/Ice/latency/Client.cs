// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client : Ice.Application
{
    public override int
    run(string[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        string refProperty = "Latency.Ping";
        string @ref = properties.getProperty(refProperty);
        if(@ref.Length == 0)
        {
            Console.Error.WriteLine("property `" + refProperty + "' not set");
            return 1;
        }
        
        PingPrx ping = PingPrxHelper.checkedCast(communicator().stringToProxy(@ref));
        if(ping == null)
        {
            Console.Error.WriteLine("invalid proxy");
            return 1;
        }
        
        // Initial ping to setup the connection.
        ping.ice_ping();
        
        long tv1 = (System.DateTime.Now.Ticks - 621355968000000000) / 10000;
        int repetitions = 100000;
        Console.Out.WriteLine("pinging server " + repetitions + " times (this may take a while)");
        for (int i = 0; i < repetitions; i++)
        {
            ping.ice_ping();
        }
        
        long tv2 = (System.DateTime.Now.Ticks - 621355968000000000) / 10000;
        double total = (double)(tv2 - tv1);
        double perPing = total / repetitions;
        
        Console.Out.WriteLine("time for " + repetitions + " pings: " + total + "ms");
        Console.Out.WriteLine("time per ping: " + perPing + "ms");
        
        return 0;
    }

    public static void Main(string[] args)
    {
        Client app = new Client();
        int status = app.main(args, "config");
        System.Environment.Exit(status);
    }
}
