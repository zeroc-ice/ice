// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using Demo;

public class Subscriber : Ice.Application
{
    public class ClockI : ClockDisp_
    {
        public override void tick(string date, Ice.Current current)
	{
	    System.Console.Out.WriteLine(date);
	}
    }

    public override int run(string[] args)
    {
        Ice.Properties properties = communicator().getProperties();

        const string proxyProperty = "IceStorm.TopicManager.Proxy";
        string proxy = properties.getProperty(proxyProperty);
        if(proxy == null)
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
        catch(IceStorm.NoSuchTopic e)
        {
            try
            {
                topic = manager.create(topicName);
            }
            catch(IceStorm.TopicExists ex)
            {
                Console.WriteLine("temporary error. try again.");
                return 1;
            }
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Clock.Subscriber");

        //
        // Add a Servant for the Ice Object.
        //
        Ice.ObjectPrx subscriber = adapter.addWithUUID(new ClockI());

        //
	// This demo requires no quality of service, so it will use
	// the defaults.
        //
	IceStorm.QoS qos = new IceStorm.QoS();

        topic.subscribe(qos, subscriber);
        adapter.activate();

        shutdownOnInterrupt();
        communicator().waitForShutdown();

        //
        // Unsubscribe all subscribed objects.
        //
        topic.unsubscribe(subscriber);

        return 0;
    }

    public static void Main(string[] args)
    {
        Subscriber app = new Subscriber();
        int status = app.main(args, "config.sub");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
