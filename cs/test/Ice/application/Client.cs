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

public class Client : Ice.Application
{
    public override void interruptCallback(int sig)
    {
        Console.WriteLine("handling signal " + sig);
    }

    public override int run(string[] args)
    {
        ignoreInterrupt();
        Console.WriteLine("Ignore CTRL+C and the like for 5 seconds (try it!)");
        System.Threading.Thread.Sleep(5 * 1000);

        callbackOnInterrupt();

        holdInterrupt();
        Console.WriteLine("Hold CTRL+C and the like for 5 seconds (try it!)");
        System.Threading.Thread.Sleep(5 * 1000);

        releaseInterrupt();
        Console.WriteLine("Release CTRL+C (any held signals should be released)");
        System.Threading.Thread.Sleep(5 * 1000);

        holdInterrupt();
        Console.WriteLine("Hold CTRL+C and the like for 5 seconds (try it!)");
        System.Threading.Thread.Sleep(5 * 1000);

        callbackOnInterrupt();
        Console.WriteLine("Release CTRL+C (any held signals should be released)");
        System.Threading.Thread.Sleep(5 * 1000);

        shutdownOnInterrupt();
        Console.WriteLine("Test shutdown on destroy. Press CTRL+C to shutdown & terminate");
        communicator().waitForShutdown();

        Console.WriteLine("ok");
        return 0;
    }

    public static void Main(string[] args)
    {
        Debug.Listeners.Add(new ConsoleTraceListener());

        Client app = new Client();
        int status = app.main(args);
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
