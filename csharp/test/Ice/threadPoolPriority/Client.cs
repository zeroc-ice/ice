// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

public class Client : TestCommon.Application
{
    public override int run(string[] args)
    {
        Console.Out.Write("testing server priority... ");
        Console.Out.Flush();
        Ice.ObjectPrx obj = communicator().stringToProxy("test:" + getTestEndpoint(0) + " -t 10000");
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
        Client app = new Client();
        return app.runmain(args);
    }
}

