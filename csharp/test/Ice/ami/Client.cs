// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
        AllTests.allTests(communicator, false);
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
            initData.properties.setProperty("Ice.Warn.AMICallback", "0");
#if COMPACT
            //
            // When using Ice for .NET Compact Framework, we need to specify
            // the assembly so that Ice can locate classes and exceptions.
            //
            initData.properties.setProperty("Ice.FactoryAssemblies", "client");
#endif
            //
            // Limit the send buffer size, this test relies on the socket
            // send() blocking after sending a given amount of data.
            //
            initData.properties.setProperty("Ice.TCP.SndSize", "50000");

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
