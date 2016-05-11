// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    private static int run(string[] args, Ice.Communicator communicator)
    {
        Timer timer = new Timer();

        communicator.getProperties().setProperty("TestAdapter1.Endpoints", "default -p 12010:udp");
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Size", "5");
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
        communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Serialize", "0");
        Ice.ObjectAdapter adapter1 = communicator.createObjectAdapter("TestAdapter1");
        adapter1.add(new HoldI(timer, adapter1), communicator.stringToIdentity("hold"));

        communicator.getProperties().setProperty("TestAdapter2.Endpoints", "default -p 12011:udp");
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Size", "5");
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
        communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Serialize", "1");
        Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
        adapter2.add(new HoldI(timer, adapter2), communicator.stringToIdentity("hold"));

        adapter1.activate();
        adapter2.activate();

        communicator.waitForShutdown();

        timer.shutdown();
        timer.waitForShutdown();

        return 0;
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            communicator = Ice.Util.initialize(ref args);
            status = run(args, communicator);
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine(ex);
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
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        return status;
    }
}
