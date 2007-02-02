// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        System.out.println("Usage: " + appName() + " [--batch] [--datagram|--twoway|--ordered|--oneway] [topic]");
    }

    public int
    run(String[] args)
    {
        IceStorm.TopicManagerPrx manager = IceStorm.TopicManagerPrxHelper.checkedCast(
            communicator().propertyToProxy("IceStorm.TopicManager.Proxy"));
        if(manager == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        String topicName = "time";
        boolean datagram = false;
        boolean twoway = false;
        boolean ordered = false;
        boolean batch = false;
        int optsSet = 0;
        for(int i = 0; i < args.length; ++i)
        {
            if(args[i].equals("--datagram"))
            {
                datagram = true;
                ++optsSet;
            }
            else if(args[i].equals("--twoway"))
            {
                twoway = true;
                ++optsSet;
            }
            else if(args[i].equals("--ordered"))
            {
                ordered = true;
                ++optsSet;
            }
            else if(args[i].equals("--oneway"))
            {
                ++optsSet;
            }
            else if(args[i].equals("--batch"))
            {
                batch = true;
            }
            else if(args[i].startsWith("--"))
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
            System.err.println(appName() + ": batch can only be set with oneway or datagram");
            return 1;
        }

        if(optsSet > 1)
        {
            usage();
            return 1;
        }

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
        // Add a Servant for the Ice Object.
        //
        java.util.Map qos = new java.util.HashMap();
        Ice.ObjectPrx subscriber = adapter.addWithUUID(new ClockI());
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
            qos.put("reliability", "ordered");
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
        
        try
        {
            topic.subscribeAndGetPublisher(qos, subscriber);
        }
        catch(IceStorm.AlreadySubscribed e)
        {
            e.printStackTrace();
            return 1;
        }
        catch(IceStorm.BadQoS e)
        {
            e.printStackTrace();
            return 1;
        }
        adapter.activate();

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
