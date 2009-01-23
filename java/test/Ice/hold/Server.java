// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        java.util.Timer timer = new java.util.Timer();

        communicator.getProperties().setProperty("TestAdapter1.Endpoints", "default -p 12010 -t 10000:udp");
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Size", "5");
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Serialize", "0");
        Ice.ObjectAdapter adapter1 = communicator.createObjectAdapter("TestAdapter1");
        adapter1.add(new HoldI(timer, adapter1), communicator.stringToIdentity("hold"));

        communicator.getProperties().setProperty("TestAdapter2.Endpoints", "default -p 12011 -t 10000:udp");
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Size", "5");
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Serialize", "1");
        Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
        adapter2.add(new HoldI(timer, adapter2), communicator.stringToIdentity("hold"));

        adapter1.activate();
        adapter2.activate();

        communicator.waitForShutdown();

        timer.cancel();

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            communicator = Ice.Util.initialize(args);
            status = run(args, communicator);
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.gc();
        System.exit(status);
    }
}
