// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Subscriber extends Ice.Application
{
    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();

        final String proxyProperty = "IceStorm.TopicManager.Proxy";
        String proxy = properties.getProperty(proxyProperty);
        if(proxy == null)
        {
	    System.err.println("property `" + proxyProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx base = communicator().stringToProxy(proxy);
        IceStorm.TopicManagerPrx manager = IceStorm.TopicManagerPrxHelper.checkedCast(base);
        if(manager == null)
        {
	    System.err.println("invalid proxy");
            return 1;
        }

        //
        // Gather the set of topics to which to subscribe. It is either
        // the set provided on the command line, or the topic "time".
        //
	java.util.List topics = new java.util.ArrayList();;
        if(args.length > 1)
        {
            for(int i = 0; i < args.length; ++i)
            {
                topics.add(args[i]);
            }
        }
        else
        {
            topics.add("time");
        }

        //
        // Set the requested quality of service "reliability" =
        // "batch". This tells IceStorm to send events to the subscriber
        // in batches at regular intervals.
        //
	java.util.Map qos = new java.util.HashMap();
	qos.put("reliability", "batch");

        //
        // Create the servant to receive the events.
        //
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Clock.Subscriber");
        Ice.Object clock = new ClockI();

        //
        // List of all subscribers.
        //
        java.util.Map subscribers = new java.util.HashMap();;

        //
        // Add the servant to the adapter for each topic. A ServantLocator
        // could have been used for the same purpose.
        //
	java.util.Iterator p = topics.iterator();
	while(p.hasNext())
        {
	    String name = (String)p.next();

            //
            // Add a Servant for the Ice Object.
            //
            Ice.ObjectPrx object = adapter.addWithUUID(clock);
            try
            {
                IceStorm.TopicPrx topic = manager.retrieve(name);
                topic.subscribe(qos, object);
            }
            catch(IceStorm.NoSuchTopic e)
            {
	        System.err.println(e + " name: " + e.name);
                break;
            }

            //
            // Add to the set of subscribers _after_ subscribing. This
            // ensures that only subscribed subscribers are unsubscribed
            // in the case of an error.
            //
            subscribers.put(name, object);
        }

        //
        // Unless there is a subscriber per topic then there was some
        // problem. If there was an error the application should terminate
        // without accepting any events.
        //
        if(subscribers.size() == topics.size())
        {
            adapter.activate();
            shutdownOnInterrupt();
            communicator().waitForShutdown();
        }

        //
        // Unsubscribe all subscribed objects.
        //
	p = subscribers.entrySet().iterator();
	while(p.hasNext())
        {
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();

            try
            {
                IceStorm.TopicPrx topic = manager.retrieve((String)entry.getKey());
                topic.unsubscribe((Ice.ObjectPrx)entry.getValue());
            }
            catch(IceStorm.NoSuchTopic e)
            {
	        System.err.println(e + " name: " + e.name);
            }
        }

        return 0;
    }

    public static void
    main(String[] args)
    {
        Subscriber app = new Subscriber();
        int status = app.main("Subscriber", args, "config");
        System.exit(status);
    }
}
