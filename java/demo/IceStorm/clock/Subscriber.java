// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
    
    public void
    usage()
    {
        System.out.println("Usage: " + appName() + " [--batch] [--datagram|--twoway|--ordered|--oneway] " +
                           "[--retryCount count] [--id id] [topic]");
    }

    public int
    run(String[] args)
    {
        args = communicator().getProperties().parseCommandLineOptions("Clock", args);

        String topicName = "time";
        String option = "None";
        boolean batch = false;
        String id = null;
        String retryCount = null;
        int i;
        for(i = 0; i < args.length; ++i)
        {
            String oldoption = option;
            if(args[i].equals("--datagram"))
            {
                option = "Datagram";
            }
            else if(args[i].equals("--twoway"))
            {
                option = "Twoway";
            }
            else if(args[i].equals("--ordered"))
            {
                option = "Ordered";
            }
            else if(args[i].equals("--oneway"))
            {
                option = "Oneway";
            }
            else if(args[i].equals("--batch"))
            {
                batch = true;
            }
            else if(args[i].equals("--id"))
            {
                ++i;
                if(i >= args.length)
                {
                    usage();
                    return 1;
                }
                id = args[i];
            }
            else if(args[i].equals("--retryCount"))
            {
                ++i;
                if(i >= args.length)
                {
                    usage();
                    return 1;
                }
                retryCount = args[i];
            }
            else if(args[i].startsWith("--"))
            {
                usage();
                return 1;
            }
            else
            {
                topicName = args[i++];
                break;
            }

            if(!oldoption.equals(option) && !oldoption.equals("None"))
            {
                usage();
                return 1;
            }
        }

        if(i != args.length)
        {
            usage();
            return 1;
        }

        if(retryCount != null)
        {
            if(option.equals("None"))
            {
                option = "Twoway";
            }
            else if(!option.equals("Twoway") && !option.equals("Ordered"))
            {
                usage();
                return 1;
            }
        }
        
        if(batch && (option.equals("Twoway") || option.equals("Ordered")))
        {
            System.err.println(appName() + ": batch can only be set with oneway or datagram");
            return 1;
        }

        IceStorm.TopicManagerPrx manager = IceStorm.TopicManagerPrxHelper.checkedCast(
            communicator().propertyToProxy("TopicManager.Proxy"));
        if(manager == null)
        {
            System.err.println("invalid proxy");
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
        catch(IceStorm.NoSuchTopic e)
        {
            try
            {
                topic = manager.create(topicName);
            }
            catch(IceStorm.TopicExists ex)
            {
                System.err.println(appName() + ": temporary failure, try again.");
                return 1;
            }
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Clock.Subscriber");

        //
        // Add a servant for the Ice object. If --id is used the
        // identity comes from the command line, otherwise a UUID is
        // used.
        //
        // id is not directly altered since it is used below to detect
        // whether subscribeAndGetPublisher can raise
        // AlreadySubscribed.
        //
        Ice.Identity subId = new Ice.Identity(id, "");
        if(subId.name == null)
        {
            subId.name = java.util.UUID.randomUUID().toString();
        }
        Ice.ObjectPrx subscriber = adapter.add(new ClockI(), subId);

        //
        // Activate the object adapter before subscribing.
        //
        adapter.activate();

        java.util.Map<String, String> qos = new java.util.HashMap<String, String>();
        if(retryCount != null)
        {
            qos.put("retryCount", retryCount);
        }
        //
        // Set up the proxy.
        //
        if(option.equals("Datagram"))
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
        else if(option.equals("Twoway"))
        {
            // Do nothing to the subscriber proxy. Its already twoway.
        }
        else if(option.equals("Ordered"))
        {
            // Do nothing to the subscriber proxy. Its already twoway.
            qos.put("reliability", "ordered");
        }
        else if(option.equals("Oneway") || option.equals("None"))
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
        catch(IceStorm.AlreadySubscribed e)
        {
            // If we're manually setting the subscriber id ignore.
            if(id == null)
            {
                e.printStackTrace();
                return 1;
            }
            else
            {
                System.out.println("reactivating persistent subscriber");
            }
        }
        catch(IceStorm.BadQoS e)
        {
            e.printStackTrace();
            return 1;
        }

        shutdownOnInterrupt();
        communicator().waitForShutdown();

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
