// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Publisher extends Ice.Application
{
    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();

        final String proxyProperty = "IceStorm.TopicManager.Proxy";
        String proxy = properties.getProperty(proxyProperty);
        if(proxy.length() == 0)
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

	//
	// Get the topic's publisher object, the Clock type, and create a
	// oneway Clock proxy (for efficiency reasons).
	//
	ClockPrx clock = ClockPrxHelper.uncheckedCast(topic.getPublisher().ice_oneway());

        System.out.println("publishing tick events. Press ^C to terminate the application.");
	try
	{
	    java.text.SimpleDateFormat date = new java.text.SimpleDateFormat("MM/dd/yy HH:mm:ss:SSS");
	    while(true)
	    {
	        
                clock.tick(date.format(new java.util.Date()));

		try
		{
		    Thread.currentThread().sleep(1000);
		}
		catch(java.lang.InterruptedException e)
		{
		}
	    }
	}
	catch(Ice.CommunicatorDestroyedException ex)
	{
	    // Ignore
	}

        return 0;
    }

    public static void
    main(String[] args)
    {
        Publisher app = new Publisher();
        int status = app.main("Publisher", args, "config.pub");
        System.exit(status);
    }
}
