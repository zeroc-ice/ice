// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

public class Collocated
{
    private static int
    run(string[] args, Ice.Communicator communicator)
    {
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010 -t 2000");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object obj = new InitialI(adapter);
        adapter.add(obj, communicator.stringToIdentity("initial"));
        AllTests.allTests(communicator);
        return 0;
    }

    public static void
    Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;
        
        Debug.Listeners.Add(new ConsoleTraceListener());

        try
        {
            communicator = Ice.Util.initialize(ref args);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
            System.Console.WriteLine(ex);
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
                System.Console.WriteLine(ex);
                status = 1;
            }
        }
        
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
