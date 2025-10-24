// Copyright (c) ZeroC, Inc.

using System.Globalization;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceDiscoveryTest")]
[assembly: AssemblyDescription("IceDiscovery test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

namespace IceDiscovery.simple;

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        using Ice.Communicator communicator = initialize(ref args);
        int num = 0;
        try
        {
            num = int.Parse(args[0], CultureInfo.InvariantCulture);
        }
        catch (FormatException)
        {
        }

        communicator.getProperties().setProperty("ControlAdapter.Endpoints", getTestEndpoint(num));
        communicator.getProperties().setProperty("ControlAdapter.AdapterId", "control" + num);
        communicator.getProperties().setProperty("ControlAdapter.ThreadPool.Size", "1");

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
        adapter.add(new ControllerI(), Ice.Util.stringToIdentity("controller" + num));
        adapter.activate();

        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Server>(args);
}
