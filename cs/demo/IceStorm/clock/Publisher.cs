// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Globalization;
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

        IceStorm.TopicManagerPrx manager = IceStorm.TopicManagerPrxHelper.checkedCast(
	    communicator().stringToProxy(proxy));
        if(manager == null)
        {
            Console.WriteLine("invalid proxy");
            return 1;
        }

	string topicName = "time";
	if(args.Length != 0)
	{
	    topicName = args[0];
	}

        //
        // Retrieve the topic.
        //
        IceStorm.TopicPrx topic;
        try
        {
            topic = manager.retrieve(topicName);
        }
        catch(IceStorm.NoSuchTopic)
        {
	    try
	    {
                topic = manager.create(topicName);
	    }
	    catch(IceStorm.TopicExists)
	    {
                Console.WriteLine("temporary error. try again.");
                return 1;
	    }
        }

        //
        // Get the topic's publisher object, the Clock type, and create a 
	// oneway Clock proxy (for efficiency reasons).
        //
	ClockPrx clock = ClockPrxHelper.uncheckedCast(topic.getPublisher().ice_oneway());

        Console.WriteLine("publishing tick events. Press ^C to terminate the application.");
	try
	{
            while(true)
            {
                clock.tick(System.DateTime.Now.ToString("G", DateTimeFormatInfo.InvariantInfo));

		System.Threading.Thread.Sleep(1000);
            }
	}
	catch(Ice.CommunicatorDestroyedException)
	{
	    // Ignore
	}

        return 0;
    }

    public static void Main(string[] args)
    {
        Publisher app = new Publisher();
        int status = app.main(args, "config.pub");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
