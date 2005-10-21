// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using Demo;

public class Publisher : Ice.Application
{
    public override int run(string[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        const String proxyProperty = "IceStorm.TopicManager.Proxy";
        String proxy = properties.getProperty(proxyProperty);
        if(proxy.Length == 0)
        {
            Console.WriteLine("property `" + proxyProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx basePrx = communicator().stringToProxy(proxy);
        IceStorm.TopicManagerPrx manager = IceStorm.TopicManagerPrxHelper.checkedCast(basePrx);
        if(manager == null)
        {
            Console.WriteLine("invalid proxy");
            return 1;
        }

        //
        // Retrieve the topic named "time".
        //
        IceStorm.TopicPrx topic;
        try
        {
            topic = manager.retrieve("time");
        }
        catch(IceStorm.NoSuchTopic e)
        {
            Console.WriteLine(e + "name: " + e.name);
            return 1;
        }
        Debug.Assert(topic != null);

        //
        // Get the topic's publisher object, verify that it supports
        // the Clock type, and create a oneway Clock proxy (for efficiency
        // reasons).
        //
        Ice.ObjectPrx obj = topic.getPublisher();
        if(!obj.ice_isDatagram())
        {
            obj = obj.ice_oneway();
        }
        ClockPrx clock = ClockPrxHelper.uncheckedCast(obj);

        Console.WriteLine("publishing 10 tick events");
        for(int i = 0; i < 10; ++i)
        {
            clock.tick();
        }

        return 0;
    }

    public static void Main(string[] args)
    {
        Publisher app = new Publisher();
        int status = app.main(args, "config");
        System.Environment.Exit(status);
    }
}
