// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

	Ice.ObjectPrx base = communicator().stringToProxy(proxy);
        IceStorm.TopicManagerPrx manager = IceStorm.TopicManagerPrxHelper.checkedCast(base);
        if(manager == null)
        {
            System.err.println("invalid proxy");
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
	    System.err.println(e + "name: " + e.name);
	    return 1;
	}
	assert(topic != null);

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

        System.out.println("publishing 10 tick events");
        for(int i = 0; i < 10; ++i)
        {
            clock.tick();
        }

        return 0;
    }

    public static void
    main(String[] args)
    {
        Publisher app = new Publisher();
        int status = app.main("Publisher", args, "config");
        System.exit(status);
    }
}
