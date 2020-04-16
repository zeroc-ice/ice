//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;

[assembly: AssemblyTitle("IceDiscoveryTest")]
[assembly: AssemblyDescription("IceDiscovery test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void Run(string[] args)
    {
        using var communicator = Initialize(ref args);
        int num;
        try
        {
            num = args.Length == 1 ? int.Parse(args[0]) : 0;
        }
        catch (FormatException)
        {
            num = 0;
        }
        AllTests.allTests(this, num);
    }

    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);
}
