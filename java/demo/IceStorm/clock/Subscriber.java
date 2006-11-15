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
    public class ClockI extends _ClockDisp
    {
        public void
	tick(String date, Ice.Current current)
	{
	    System.out.println(date);
	}
    }

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

        IceStorm.TopicManagerPrx manager = IceStorm.TopicManagerPrxHelper.checkedCast(
	    communicator().stringToProxy(proxy));
        if(manager == null)
        {
	    System.err.println("invalid proxy");
            return 1;
        }

        String topicName = "time";
        if(args.length != 0)
        {
            topicName = args[0];
        }

        //
        // Retrieve the topic named "time".
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
                System.err.println("temporary failure, try again.");
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
	java.util.Map qos = new java.util.HashMap();

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

    public static void
    main(String[] args)
    {
        Subscriber app = new Subscriber();
        int status = app.main("Subscriber", args, "config.sub");
        System.exit(status);
    }
}
