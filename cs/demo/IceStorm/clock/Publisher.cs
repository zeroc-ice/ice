// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Diagnostics;
using System.Globalization;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceStormClockPublisher")]
[assembly: AssemblyDescription("IceStorm clock demo publisher")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Publisher
{
    public class App : Ice.Application
    {
        public override int run(string[] args)
        {
            string option = "None";
            string topicName = "time";
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
                else if(args[i].Equals("--oneway"))
                {
                    option = "Oneway";
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

            //
            // Get the topic's publisher object, and create a Clock proxy with
            // the mode specified as an argument of this application.
            //
            Ice.ObjectPrx publisher = topic.getPublisher();
            if(option.Equals("Datagram"))
            {
                publisher = publisher.ice_datagram();
            }
            else if(option.Equals("Twoway"))
            {
                // Do nothing.
            }
            else // if(oneway)
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
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.pub");
    }
}
