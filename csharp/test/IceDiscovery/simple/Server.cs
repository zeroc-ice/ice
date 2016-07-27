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

[assembly: AssemblyTitle("IceDiscoveryTest")]
[assembly: AssemblyDescription("IceDiscovery test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    private static int run(string[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();

        int num = 0;
        try
        {
            num =  Int32.Parse(args[0]);
        }
        catch(FormatException)
        {
        }

        properties.setProperty("ControlAdapter.Endpoints", "default -p " + (12010 + num));
        properties.setProperty("ControlAdapter.AdapterId", "control" + num);
        properties.setProperty("ControlAdapter.ThreadPool.Size", "1");

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
        adapter.add(new ControllerI(), Ice.Util.stringToIdentity("controller" + num));
        adapter.activate();

        communicator.waitForShutdown();
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
