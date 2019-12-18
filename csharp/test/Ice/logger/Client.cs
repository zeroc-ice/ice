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
    public override void run(string[] args)
    {
        Console.Out.Write("testing Ice.LogFile... ");
        Console.Out.Flush();
        if (File.Exists("log.txt"))
        {
            File.Delete("log.txt");
        }

        {
            var properties = createTestProperties(ref args);
            properties["Ice.LogFile"] = "log.txt";
            using var communicator = initialize(properties);
            communicator.Logger.trace("info", "my logger");
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
