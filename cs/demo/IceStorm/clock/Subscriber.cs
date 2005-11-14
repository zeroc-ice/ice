// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

        Ice.ObjectPrx basePrx = communicator().stringToProxy(proxy);
        IceStorm.TopicManagerPrx manager = IceStorm.TopicManagerPrxHelper.checkedCast(basePrx);
        if(manager == null)
        {
            Console.WriteLine("invalid proxy");
            return 1;
        }

        //
        // Gather the set of topics to which to subscribe. It is either
        // the set provided on the command line, or the topic "time".
        //
        ArrayList topics = new ArrayList();;
        if(args.Length > 1)
        {
            for(int i = 0; i < args.Length; ++i)
            {
                topics.Add(args[i]);
            }
        }
        else
        {
            topics.Add("time");
        }

        //
        // Set the requested quality of service "reliability" =
        // "batch". This tells IceStorm to send events to the subscriber
        // in batches at regular intervals.
        //
	IceStorm.QoS qos = new IceStorm.QoS();
        qos["reliability"] = "batch";

        //
        // Create the servant to receive the events.
        //
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Clock.Subscriber");
        Ice.Object clock = new ClockI();

        //
        // List of all subscribers.
        //
	Hashtable subscribers = new Hashtable();

        //
        // Add the servant to the adapter for each topic. A ServantLocator
        // could have been used for the same purpose.
        //
	for(int i = 0; i < topics.Count; ++i)
        {
            //
            // Add a Servant for the Ice Object.
            //
            Ice.ObjectPrx obj = adapter.addWithUUID(clock);
            try
            {
                IceStorm.TopicPrx topic = manager.retrieve((string)topics[i]);
                topic.subscribe(qos, obj);
            }
            catch(IceStorm.NoSuchTopic e)
            {                               
                Console.WriteLine(e + " name: " + e.name);
            }

            //
            // Add to the set of subscribers _after_ subscribing. This
            // ensures that only subscribed subscribers are unsubscribed
            // in the case of an error.
            //
            subscribers[(string)topics[i]] = obj;
        }

        //
        // Unless there is a subscriber per topic then there was some
        // problem. If there was an error the application should terminate
        // without accepting any events.
        //
        if(subscribers.Count == topics.Count)
        {
            adapter.activate();
            shutdownOnInterrupt();
            communicator().waitForShutdown();
        }

        //
        // Unsubscribe all subscribed objects.
        //
	foreach(DictionaryEntry entry in (Hashtable)subscribers)
        {
            try
            {
                IceStorm.TopicPrx topic = manager.retrieve((string)entry.Key);
                topic.unsubscribe((Ice.ObjectPrx)entry.Value);
            }
            catch(IceStorm.NoSuchTopic e)
            {
                Console.WriteLine(e + " name: " + e.name);
            }
        }

        return 0;
    }

    public static void Main(string[] args)
    {
        Subscriber app = new Subscriber();
        int status = app.main(args, "config");
        System.Environment.Exit(status);
    }
}
