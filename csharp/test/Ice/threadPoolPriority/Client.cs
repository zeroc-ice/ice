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

public class Client
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    private static int run(string[] args, Ice.Communicator communicator)
    {
        Console.Out.Write("testing server priority... ");
        Console.Out.Flush();
        Ice.ObjectPrx obj = communicator.stringToProxy("test:default -p 12010 -t 10000");
        Test.PriorityPrx priority = Test.PriorityPrxHelper.checkedCast(obj);

        try
        {
            test("AboveNormal".Equals(priority.getPriority()));
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine(ex.ToString());
            test(false);
        }
        Console.Out.WriteLine("ok");

        priority.shutdown();
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

