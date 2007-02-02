// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        IceStorm.TopicManagerPrx manager = IceStorm.TopicManagerPrxHelper.checkedCast(
            communicator().propertyToProxy("IceStorm.TopicManager.Proxy"));
        if(manager == null)
        {
            Console.WriteLine("invalid proxy");
            return 1;
        }

        string topicName = "time";
        bool datagram = false;
        bool twoway = false;
        bool ordered = false;
        bool batch = false;
        int optsSet = 0;
        for(int i = 0; i < args.Length; ++i)
        {
            if(args[i].Equals("--datagram"))
            {
                datagram = true;
                ++optsSet;
            }
            else if(args[i].Equals("--twoway"))
            {
                twoway = true;
                ++optsSet;
            }
            else if(args[i].Equals("--ordered"))
            {
                ordered = true;
                ++optsSet;
            }
            else if(args[i].Equals("--oneway"))
            {
                ++optsSet;
            }
            else if(args[i].Equals("--batch"))
            {
                batch = true;
            }
            else if(args[i].StartsWith("--"))
            {
                usage();
                return 1;
            }
            else
            {
                topicName = args[i];
                break;
            }
        }

        if(batch && (twoway || ordered))
	{
	    Console.WriteLine(appName() + ": batch can only be set with oneway or datagram");
	    return 1;
	}

        if(optsSet > 1)
        {
            usage();
            return 1;
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

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Clock.Subscriber");

        //
        // Add a Servant for the Ice Object.
        //
        Ice.ObjectPrx subscriber = adapter.addWithUUID(new ClockI());

        IceStorm.QoS qos = new IceStorm.QoS();

        //
        // Set up the proxy.
        //
        if(datagram)
        {
            subscriber = subscriber.ice_datagram();
        }
        else if(twoway)
        {
            // Do nothing to the subscriber proxy. Its already twoway.
        }
        else if(ordered)
        {
            // Do nothing to the subscriber proxy. Its already twoway.
            qos["reliability"] = "ordered";
        }
        else // if(oneway)
        {
            subscriber = subscriber.ice_oneway();
        }
        if(batch)
        {
            if(datagram)
            {
                subscriber = subscriber.ice_batchDatagram();
            }
            else
            {
                subscriber = subscriber.ice_batchOneway();
            }
        }
        
        topic.subscribeAndGetPublisher(qos, subscriber);
        adapter.activate();

        shutdownOnInterrupt();
        communicator().waitForShutdown();

        //
        // Unsubscribe all subscribed objects.
        //
        topic.unsubscribe(subscriber);

        return 0;
    }

    public void
    usage()
    {
        Console.WriteLine("Usage: " + appName() + " [--batch] [--datagram|--twoway|--ordered|--oneway] [topic]");
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
