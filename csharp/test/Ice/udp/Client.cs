// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
using System.Diagnostics;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    private static int run(string[] args, Ice.Communicator communicator)
    {
        AllTests.allTests(communicator);

        int num;
        try
        {
            num = args.Length == 1 ? System.Int32.Parse(args[0]) : 0;
        }
        catch(System.FormatException)
        {
            num = 0;
        }
        for(int i = 0; i < num; ++i)
        {
            TestIntfPrxHelper.uncheckedCast(communicator.stringToProxy("control:tcp -p " + (12010 + i))).shutdown();
        }
        return 0;
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);

            initData.properties.setProperty("Ice.Warn.Connections", "0");
            initData.properties.setProperty("Ice.UDP.SndSize", "16384");

            communicator = Ice.Util.initialize(ref args, initData);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
            System.Console.Error.WriteLine(ex);
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
                System.Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        return status;
    }
}
