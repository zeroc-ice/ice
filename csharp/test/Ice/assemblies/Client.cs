//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using Test;

using Ice;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : TestHelper
{
    public override void Run(string[] args)
    {
        Console.Out.Write("testing preloading assemblies... ");
        Console.Out.Flush();
        var info = new User.UserInfo();

        Dictionary<string, string> properties = CreateTestProperties(ref args);
        properties["Ice.PreloadAssemblies"] = "0";

        string assembly =
            $"{Path.GetFileName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase))}/core.dll";

        using (Communicator communicator = Initialize(properties))
        {
            Assert(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(
                e => e.CodeBase!.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase)) == null);
        }
        properties["Ice.PreloadAssemblies"] = "1";
        using (Communicator communicator = Initialize(properties))
        {
            Assert(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(
                e => e.CodeBase!.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase)) != null);
        }

        Console.Out.WriteLine("ok");
    }

    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);
}
