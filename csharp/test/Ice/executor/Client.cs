// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        try
        {
            var initData = new Ice.InitializationData();
            initData.properties = createTestProperties(ref args);
            initData.properties.setProperty("Ice.Warn.AMICallback", "0");
            //
            // Limit the send buffer size, this test relies on the socket
            // send() blocking after sending a given amount of data.
            //
            initData.properties.setProperty("Ice.TCP.SndSize", "50000");
            initData.executor = new Executor().execute;
            using var communicator = initialize(initData);
            AllTests.allTests(this);
        }
        finally
        {
            Executor.terminate();
        }
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
