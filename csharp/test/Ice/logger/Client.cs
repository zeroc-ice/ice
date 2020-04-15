//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;
using System.IO;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void Run(string[] args)
    {
        Console.Out.Write("testing Ice.LogFile... ");
        Console.Out.Flush();
        if (File.Exists("log.txt"))
        {
            File.Delete("log.txt");
        }

        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.LogFile"] = "log.txt";
            using var communicator = Initialize(properties);
            communicator.Logger.Trace("info", "my logger");
        }
        Assert(File.Exists("log.txt"));
        Assert(File.ReadAllText("log.txt").Contains("my logger"));
        File.Delete("log.txt");
        Console.Out.WriteLine("ok");
    }

    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);
}
