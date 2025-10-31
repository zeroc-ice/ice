// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

namespace Ice.logger;

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
        var initData = new Ice.InitializationData();
        initData.properties = createTestProperties(ref args);
        initData.properties.setProperty("Ice.LogFile", "log.txt");
        using (Communicator communicator = initialize(initData))
        {
            communicator.getLogger().trace("info", "my logger");
        }
        test(File.Exists("log.txt"));
        test(File.ReadAllText("log.txt").Contains("my logger", StringComparison.Ordinal));
        File.Delete("log.txt");
        Console.Out.WriteLine("ok");
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
