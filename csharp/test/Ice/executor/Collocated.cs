// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Collocated : Test.TestHelper
{
    public override void run(string[] args)
    {
        try
        {
            var initData = new Ice.InitializationData();
            initData.properties = createTestProperties(ref args);
            initData.properties.setProperty("Ice.Warn.AMICallback", "0");
            initData.executor = new Executor().execute;

            using var communicator = initialize(initData);
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

            adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
            // Don't activate OA to ensure collocation is used.
            adapter2.add(new TestControllerI(adapter), Ice.Util.stringToIdentity("testController"));
            // Don't activate OA to ensure collocation is used.

            AllTests.allTests(this);
        }
        finally
        {
            Executor.terminate();
        }
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Collocated>(args);
}
