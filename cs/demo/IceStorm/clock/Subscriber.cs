// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Collections.Generic;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceStormClockSubscriber")]
[assembly: AssemblyDescription("IceStorm clock demo subscriber")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Subscriber
{
    public class App : Ice.Application
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
            args = communicator().getProperties().parseCommandLineOptions("Clock", args);

            string topicName = "time";
            string option = "None";
            bool batch = false;
            string id = null;
            string retryCount = null;
            int i;
            for(i = 0; i < args.Length; ++i)
            {
                String oldoption = option;
                if(args[i].Equals("--datagram"))
                {
                    option = "Datagram";
                }
                else if(args[i].Equals("--twoway"))
                {
                    option = "Twoway";
                }
                else if(args[i].Equals("--ordered"))
                {
                    option = "Ordered";
                }
                else if(args[i].Equals("--oneway"))
                {
                    option = "Oneway";
                }
                else if(args[i].Equals("--batch"))
                {
                    batch = true;
                }
                else if(args[i].Equals("--id"))
                {
                    ++i;
                    if(i >= args.Length)
                    {
                        usage();
                        return 1;
                    }
                    id = args[i];
                }
                else if(args[i].Equals("--retryCount"))
                {
                    ++i;
                    if(i >= args.Length)
                    {
                        usage();
                        return 1;
                    }
                    retryCount = args[i];
                }
                else if(args[i].StartsWith("--"))
                {
                    usage();
                    return 1;
                }
                else
                {
                    topicName = args[i++];
                    break;
                }

                if(!oldoption.Equals(option) && !oldoption.Equals("None"))
                {
                    usage();
                    return 1;
                }
            }

            if(i != args.Length)
            {
                usage();
                return 1;
            }

            if(retryCount != null)
            {
                if(option.Equals("None"))
                {
                    option = "Twoway";
                }
                else if(!option.Equals("Twoway") && !option.Equals("Ordered"))
                {
                    usage();
                    return 1;
                }
            }

            if(batch && (option.Equals("Twoway") || option.Equals("Ordered")))
            {
                Console.WriteLine(appName() + ": batch can only be set with oneway or datagram");
                return 1;
            }

            IceStorm.TopicManagerPrx manager = IceStorm.TopicManagerPrxHelper.checkedCast(
                communicator().propertyToProxy("TopicManager.Proxy"));
            if(manager == null)
            {
                Console.WriteLine("invalid proxy");
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
            // Add a servant for the Ice object. If --id is used the
            // identity comes from the command line, otherwise a UUID is
            // used.
            //
            // id is not directly altered since it is used below to
            // detect whether subscribeAndGetPublisher can raise
            // AlreadySubscribed.
            //
            Ice.Identity subId = new Ice.Identity(id, "");
            if(subId.name == null)
            {
                subId.name = Guid.NewGuid().ToString();
            }
            Ice.ObjectPrx subscriber = adapter.add(new ClockI(), subId);

            //
            // Activate the object adapter before subscribing.
            //
            adapter.activate();

            Dictionary<string, string> qos = new Dictionary<string, string>();
            if(retryCount != null)
            {
                qos["retryCount"] = retryCount;
            }
            //
            // Set up the proxy.
            //
            if(option.Equals("Datagram"))
            {
                if(batch)
                {
                    subscriber = subscriber.ice_batchDatagram();
                }
                else
                {
                    subscriber = subscriber.ice_datagram();
                }
            }
            else if(option.Equals("Twoway"))
            {
                // Do nothing to the subscriber proxy. Its already twoway.
            }
            else if(option.Equals("Ordered"))
            {
                // Do nothing to the subscriber proxy. Its already twoway.
                qos["reliability"] = "ordered";
            }
            else if(option.Equals("Oneway") || option.Equals("None"))
            {
                if(batch)
                {
                    subscriber = subscriber.ice_batchOneway();
                }
                else
                {
                    subscriber = subscriber.ice_oneway();
                }
            }

            try
            {
                topic.subscribeAndGetPublisher(qos, subscriber);
            }
            catch(IceStorm.AlreadySubscribed)
            {
                // If we're manually setting the subscriber id ignore.
                if(id == null)
                {
                    throw;
                }
                System.Console.Out.WriteLine("reactivating persistent subscriber");
            }

            shutdownOnInterrupt();
            communicator().waitForShutdown();

            topic.unsubscribe(subscriber);

            return 0;
        }

        public void
        usage()
        {
            Console.WriteLine("Usage: " + appName() + " [--batch] [--datagram|--twoway|--ordered|--oneway] " +
                              "[--retryCount count] [--id id] [topic]");
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.sub");
    }
}
