// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
            else if(args[i].Equals("--oneway"))
            {
                ++optsSet;
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

        //
        // Get the topic's publisher object, and create a Clock proxy with
        // the mode specified as an argument of this application.
        //
        Ice.ObjectPrx publisher = topic.getPublisher();
        if(datagram)
        {
            publisher = publisher.ice_datagram();
        }
        else if(twoway)
        {
            // Do nothing.
        }
        else //if(oneway)
        {
            publisher = publisher.ice_oneway();
        }
        ClockPrx clock = ClockPrxHelper.uncheckedCast(publisher);

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

    public void usage()
    {
        Console.WriteLine("Usage: " + appName() + " [--datagram|--twoway|--oneway] [topic]");
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
