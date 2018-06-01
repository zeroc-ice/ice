// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;
using System.IO;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        Console.Out.Write("testing Ice.LogFile... ");
        Console.Out.Flush();
        if(File.Exists("log.txt"))
        {
            File.Delete("log.txt");
        }
        var initData = new Ice.InitializationData();
        initData.properties = createTestProperties(ref args);
        initData.properties.setProperty("Ice.LogFile", "log.txt");
        using(var communicator = initialize(initData))
        {
            communicator.getLogger().trace("info", "my logger");
        }
        test(File.Exists("log.txt"));
        test(File.ReadAllText("log.txt").Contains("my logger"));
        File.Delete("log.txt");
        Console.Out.WriteLine("ok");
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }
}
