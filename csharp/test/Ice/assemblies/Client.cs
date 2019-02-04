//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using System.IO;
using System.Reflection;
using System.Collections.Generic;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        Console.Out.Write("testing preloading assemblies... ");
        Console.Out.Flush();
        User.UserInfo info = new User.UserInfo();

        Ice.Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.PreloadAssemblies", "0");

        string assembly =
            String.Format("{0}/core.dll",
                          Path.GetFileName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase)));
        using(var communicator = initialize(properties))
        {
            test(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault((e) =>
                    {
                        return e.CodeBase.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase);
                    }) == null);
        }
        properties.setProperty("Ice.PreloadAssemblies", "1");
        using(var communicator = initialize(properties))
        {
            test(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault((e) =>
                    {
                        return e.CodeBase.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase);
                    }) != null);
        }

        Console.Out.WriteLine("ok");
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }
}
