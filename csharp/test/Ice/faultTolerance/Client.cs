// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        Ice.Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Warn.Connections", "0");
        using var communicator = initialize(properties);
        var ports = args.Select(v => int.Parse(v)).ToList();
        if (ports.Count == 0)
        {
            throw new ArgumentException("Client: no ports specified");
        }
        await AllTests.allTests(this, ports);
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
