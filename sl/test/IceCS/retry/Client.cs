// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;

public class Client
{
    public static int run(string[] args, Ice.Communicator communicator, AllTests.LoggerI logger)
    {
        AllTests.allTests(communicator, logger);
        return 0;
    }

    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

#if !SILVERLIGHT
        Debug.Listeners.Add(new ConsoleTraceListener());
#endif

        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            AllTests.LoggerI logger = new AllTests.LoggerI();
            initData.logger = logger;
            initData.properties = Ice.Util.createProperties(ref args);
            initData.properties.setProperty("Ice.Trace.Retry", "2");
            communicator = Ice.Util.initialize(ref args, initData);
            status = run(args, communicator, logger);
        }
        catch(System.Exception ex)
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

#if !SILVERLIGHT
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
#endif
    }
}
